#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_BUFFER_SIZE 100
#define N_OF_PROCESSES 100

typedef struct
{
    int id;
    int arrival;
    int duration;
    int priority;
    int enqueued;
} Process;

typedef struct
{
    Process buffer[MAX_BUFFER_SIZE];
    int front;
    int rear;
    int count;
} Queue;

Process processos[N_OF_PROCESSES];
Process processosMQ[N_OF_PROCESSES];

int current_time = 0;
int current_timePreemp = 0;
int quantum = 2;

FILE *arqTxtEscrita;

int lerArquivo()
{
    // função para ler o arquivo e armazenar os processos
    FILE *arq;
    int i = 0;
    int arrival, duration, priority;
    arq = fopen("processes.txt", "r");
    if (arq == NULL)
    {
        printf("Erro ao abrir o arquivo\n");
        return 0;
    }

    while (fscanf(arq, "%d %d %d", &arrival, &duration, &priority) != EOF)
    {
        processos[i].id = i;
        processos[i].arrival = arrival;
        processos[i].duration = duration;
        processos[i].priority = priority;

        processosMQ[i].id = i;
        processosMQ[i].arrival = arrival;
        processosMQ[i].duration = duration;
        processosMQ[i].priority = priority;
        processosMQ[i].enqueued = 0;
        i++;
    }
    fclose(arq);
    return i;
}

void enqueue(Queue *q, Process p)
{
    if (is_full(q))
    {
        printf("Erro: Buffer cheio. Não é possível enfileirar o processo.\n");
        exit(0);
    }

    q->buffer[q->rear] = p;
    q->rear = (q->rear + 1) % MAX_BUFFER_SIZE;
    q->count++;
}

Process dequeue(Queue *q)
{
    if (is_empty(q))
    {
        printf("Erro: Buffer vazio. Não é possível desenfileirar o processo.\n");
        exit(0);
    }

    Process p = q->buffer[q->front];
    q->front = (q->front + 1) % MAX_BUFFER_SIZE;
    q->count--;
    return p;
}

Process dequeuePre(Queue *q)
{
    if (is_empty(q))
    {
        printf("Erro: Buffer vazio. Não é possível desenfileirar o processo.\n");
        exit(0);
    }

    Process *p = &q->buffer[q->front];
    p->duration -= quantum;
    if (p->duration <= 0)
    {
        q->front = (q->front + 1) % MAX_BUFFER_SIZE;
        current_timePreemp += p->duration + quantum;
        q->count--;
    }
    else
    {
        current_timePreemp += quantum;
    }

    return *p;
}

int is_empty(Queue *q)
{
    return q->count == 0;
}

int is_full(Queue *q)
{
    return q->count == MAX_BUFFER_SIZE;
}

void priorityBasedScheduler(int process_count)
{
    Queue qPBS;
    qPBS.front = 0;
    qPBS.rear = 0;
    qPBS.count = 0;

    FILE *output;
    output = fopen("output.txt", "a");

    fprintf(output, "Ordem em que os processos foram executados(PBS): \n");
    int i;
    int highest_priority;
    int current_process_index;
    int notArrived;

    while (1)
    {
        highest_priority = INT_MIN;
        current_process_index = -1;
        notArrived = 1;

        for (i = 0; i < process_count; i++)
        {
            if (processos[i].priority != -1 && processos[i].priority > highest_priority)
            {
                if (processos[i].arrival <= current_time)
                {
                    current_process_index = i;
                    highest_priority = processos[i].priority;
                }
                else
                {
                    notArrived = 0;
                }
            }
        }

        if (current_process_index == -1 && notArrived)
        {
            break;
        }

        if (current_process_index >= 0)
        {
            enqueue(&qPBS, processos[current_process_index]);
            processos[current_process_index].priority = -1;
        }

        if (qPBS.count == 1)
        {
            Process p = dequeue(&qPBS);
            current_time += p.duration;
            fprintf(output, "P%d PBS", p.id);
        }
        fprintf(output, " Tempo atual: %d\n", current_time);
    }

    fclose(output);
}

void MultipleQueues(int process_count)
{

    FILE *output;
    output = fopen("output.txt", "a");

    fprintf(output, "\n\nOrdem em que os processos foram executados(MQ): \n");
    int i;
    int highest_priority;
    int current_process_index;
    int notArrived;
    int acabo;
    acabo = 0;

    Queue qMQ[3];
    for (i = 0; i < 3; i++)
    {
        qMQ[i].front = 0;
        qMQ[i].rear = 0;
        qMQ[i].count = 0;
    }

    i = 0;
    while (1)
    {
        highest_priority = INT_MIN;
        current_process_index = -1;
        notArrived = 1;

        for (i = 0; i < process_count; i++)
        {
            if (processosMQ[i].enqueued == 0 && processosMQ[i].priority != -1 && processosMQ[i].priority > highest_priority)
            {
                if (processosMQ[i].arrival <= current_timePreemp)
                {
                    current_process_index = i;
                    highest_priority = processosMQ[i].priority;
                }
                else
                {
                    notArrived = 0;
                }
            }
        }

        if (current_process_index >= 0)
        {
            if (processosMQ[current_process_index].priority >= 9)
            {
                processosMQ[current_process_index].enqueued = 1;
                enqueue(&qMQ[0], processosMQ[current_process_index]);
            }
            else if (processosMQ[current_process_index].priority >= 4)
            {
                processosMQ[current_process_index].enqueued = 1;
                enqueue(&qMQ[1], processosMQ[current_process_index]);
            }
            else if (processosMQ[current_process_index].priority >= 0)
            {
                processosMQ[current_process_index].enqueued = 1;
                enqueue(&qMQ[2], processosMQ[current_process_index]);
            }
        }

        for (i = 0; i < 3; i++)
        {
            if (qMQ[i].count >= 1)
            {
                Process p = dequeuePre(&qMQ[i]);
                fprintf(output, "P%d MQ-%d", p.id, i);
                fprintf(output, " Duracao: %d ", p.duration);
                break;
            }
            if (i == 2)
            {
                acabo = 1;
            }
        }
        fprintf(output, " Tempo atual: %d\n", current_timePreemp);
        if (acabo && notArrived)
        {
            break;
        }
    }
    fclose(output);
}

int main()
{
    //Ponteiro de função para a função:"ler"
    int (*ler)() = lerArquivo;
    int process_count = ler();

    priorityBasedScheduler(process_count);
    MultipleQueues(process_count);
    printf("Escalonamento concluído. Verifique o arquivo output.txt para os resultados.\n");

    return 0;
}
