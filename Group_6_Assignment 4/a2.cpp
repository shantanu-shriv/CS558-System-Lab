#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <iomanip>
#include <vector>

using namespace std;

#define MAX_QUESTION_SIZE 100
#define MAX_ANSWER_SIZE 10
#define MAX_STUDENTS 50

struct question
{
    long type;
    char text[20];
};

struct answer
{
    long type;
    char text[20];
};

int main()
{
    int num_students, num_questions;
    cout << "Enter the number of students: ";
    cin >> num_students;
    cout << "Enter the number of questions: ";
    cin >> num_questions;

    int qid1 = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    int qid2 = msgget(IPC_PRIVATE, IPC_CREAT | 0666);

    if (qid1 == -1 || qid2 == -1) {
        cout << "Error while creating the message queue." << endl;
        return -1;
    }

  vector<int> score_distribution(num_questions + 1);

    for (int i = 1; i <= num_students; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            cout << "Error: Failed to create child process\n";
            exit(1);
        }
        else if (pid == 0) {
            srand(time(NULL) + getpid());

            for (int j = 1; j <= num_questions; j++)
            {
                question q;
                answer a;

                msgrcv(qid1, &q, sizeof(question) - sizeof(long), i, 0);
                int rans = rand() % 4;
                printf(" Student-%d -> Question-%d -> Answer generated - %d\n", i, j, rans);

                if (rans == 0)
		{
		strncpy(a.text, "A", MAX_ANSWER_SIZE);
		}
		else if (rans == 1)
		{
		strncpy(a.text, "B", MAX_ANSWER_SIZE);
		}
		else if (rans == 2)
		{
		strncpy(a.text, "C", MAX_ANSWER_SIZE);
		}
		else if (rans == 3)
		{
		strncpy(a.text, "D", MAX_ANSWER_SIZE);
		}

                a.type = i;
                msgsnd(qid2, &a, sizeof(answer) - sizeof(long), 0);
            }

            exit(0);
        }
    }

    int grades[MAX_STUDENTS]; 
    memset(grades, 0, sizeof(grades));

    for (int i = 1; i <= num_questions; i++)
    {
        question q;

        int rans = rand() % 4;
        printf("\nQuestion-%d Correct Answer = %d\n", i, rans);

        char correct_ans[MAX_ANSWER_SIZE];
        
	if (rans == 0) {
	    strncpy(correct_ans, "A", MAX_ANSWER_SIZE);
	} else if (rans == 1) {
	    strncpy(correct_ans, "B", MAX_ANSWER_SIZE);
	} else if (rans == 2) {
	    strncpy(correct_ans, "C", MAX_ANSWER_SIZE);
	} else if (rans == 3) {
	    strncpy(correct_ans, "D", MAX_ANSWER_SIZE);
	}

        strncpy(q.text, "Q", MAX_ANSWER_SIZE);

        for (int j = 1; j <= num_students; j++) {
            q.type = j;
            if (msgsnd(qid1, &q, sizeof(question) - sizeof(long), 0) == -1)
                cout << "Message not sent";
        }

        for (int j = 1; j <= num_students; j++)
        {
            answer a;
            msgrcv(qid2, &a, sizeof(answer) - sizeof(long), 0, 0);

            if (strncmp(a.text, correct_ans, MAX_ANSWER_SIZE) == 0)
                grades[(int)a.type]++;
        }
    }

    for (int i = 0; i < num_students; i++)
    {
        int status;
        wait(&status);
        if (status != 0)
            cout << "Warning: Child process " << i << " terminated unexpectedly." << endl;
    }

    sleep(3);
    cout << "\n\n______REPORT CARD_______\n";
    for (int i = 1; i <= num_students; i++){
 score_distribution[grades[i]]++;
        cout << " Student " << i << " --> " << setprecision(4) << (double)grades[i] / num_questions * 100 << "% " << endl;
    }


    cout << "\n\nOverall Grade Distribution:\n";

    for(int i = 0; i <= num_questions; i++)
        cout << i << " correct answer: " << score_distribution[i] << " Students"<< endl;

    int deleteFlag1 = msgctl(qid1, IPC_RMID, NULL);
    int deleteFlag2 = msgctl(qid2, IPC_RMID, NULL);
    if (deleteFlag1 == -1 || deleteFlag2 == -1)
        cout << "Error while deleting message queues";

    return 0;
}
