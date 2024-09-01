
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define CACHE_SIZE 3
#define BUFFER_SIZE 4096 // Buffer size for reading HTTP responses

// Structure to represent a cached page
typedef struct CachedPage
{
    char url[256];
    char content[BUFFER_SIZE];
    struct CachedPage *next;
} CachedPage;

// Global variables
CachedPage *cache = NULL;
int cacheCount = 0;

//loading
void loadingAnimationRandomBlocks(int duration) {
    int i;
    
    for (i = 0; i < duration * 4; i++) {
        
        fflush(stdout);
        usleep(250000);  // Wait for 250 milliseconds (1/4 second)
    }
    
    printf("\rLoading... Done!  \n");
}

// Function to add a page to the cache or move it to the front if already present
void addToCache(const char *url, const char *content)
{
    CachedPage *newPage = (CachedPage *)malloc(sizeof(CachedPage));
    if (!newPage)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    strncpy(newPage->url, url, sizeof(newPage->url));
    strncpy(newPage->content, content, sizeof(newPage->content));
    newPage->next = NULL;

    // Check if the page is already in the cache
    CachedPage *current = cache;
    CachedPage *prev = NULL;
    while (current)
    {
        if (strcmp(current->url, url) == 0)
        {
            // Page found in cache, move it to the front
            if (prev)
            {
                prev->next = current->next;
                current->next = cache;
                cache = current;
            }
            free(newPage); // Free the newPage since it's not needed
            return;
        }
        prev = current;
        current = current->next;
    }

    // Page not found in cache, add it to the front
    newPage->next = cache;
    cache = newPage;
    cacheCount++;

    // If cache size exceeds the limit, remove the least recently used page
    if (cacheCount > CACHE_SIZE)
    {
        current = cache;
        prev = NULL;
        while (current->next)
        {
            prev = current;
            current = current->next;
        }
        free(current);
        if (prev)
        {
            prev->next = NULL;
        }
        else
        {
            cache = NULL;
        }
        cacheCount--;
    }
}

// Function to fetch a web page using HTTP GET request
char *fetchPage(const char *url)
{
    // Check if the page is already in the cache
    CachedPage *current = cache;
    while (current)
    {
        if (strcmp(current->url, url) == 0)
        {
            // Page found in cache, move it to the front
            addToCache(current->url, current->content);
            return strdup(current->content); // Duplicate and return the content
        }
        current = current->next;
    }

    // Page not found in cache, fetch it using sockets
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;
    char hostname[256];
    sscanf(url, "http://%255[^/]", hostname);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    // Replace with the actual hostname
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        exit(1);
    }

    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(80); // HTTP port

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting to server");
        exit(1);
    }

    // Extract the path from the URL
    char path[256];
    if (sscanf(url, "http://%*[^/]/%255s", path) != 1)
    {
        fprintf(stderr, "Invalid URL format\n");
        exit(1);
    }

    // Send HTTP GET request
    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, hostname);

    if (write(sockfd, request, strlen(request)) < 0)
    {
        perror("Error writing to socket");
        close(sockfd);
        exit(1);
    }

    // Read and store the HTTP response
    char *response = (char *)malloc(BUFFER_SIZE); // Dynamically allocate memory
    if (!response)
    {
        perror("Memory allocation error");
        exit(1);
    }
    memset(response, 0, BUFFER_SIZE);
    ssize_t totalBytes = 0;
    ssize_t bytesRead;

    while ((bytesRead = read(sockfd, response + totalBytes, BUFFER_SIZE - totalBytes - 1)) > 0)
    {
        totalBytes += bytesRead;
    }

    if (bytesRead < 0)
    {
        perror("Error reading from socket");
        close(sockfd);
        free(response); // Free the allocated memory in case of an error
        exit(1);
    }

    // Close the socket
    close(sockfd);

    // Add the fetched page to the cache
    addToCache(url, response);

    return response;
}

// Function to display the contents of the cache
void displayCache()
{
    printf("Cache Contents (Most to Least Recently Used):\n");
    CachedPage *current = cache;
    while (current)
    {
        printf("%s\n", current->url);
        current = current->next;
    }
}

int main()
{
    char input[256];
    while (1)
    {
        printf("Enter URL (or 'exit' to quit): ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
        {
            break;
        }
     

        const char *content = fetchPage(input);
        loadingAnimationRandomBlocks(5);
        printf("Page content:\n%s\n", content);
        displayCache();

        // Free the allocated memory for the response
        free((void *)content);
    }

    // Free cache memory
    while (cache)
    {
        CachedPage *temp = cache->next;
        free(cache);
        cache = temp;
    }

    return 0;
}
