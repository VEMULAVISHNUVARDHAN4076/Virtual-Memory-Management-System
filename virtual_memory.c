#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define MAX 100
#define MEMORY_SIZE 1000
// ---------- Structures ----------
typedef struct
{
    int base;
    int size;
    int allocated;
    int process_id;
} Segment;
// ---------- Paging Algorithms ----------
int simulate_fifo(int pages[], int n, int frame_count, int *hits)
{
    int memory[frame_count];
    int front = 0, faults = 0,i,j;
    *hits = 0;
    memset(memory, -1, sizeof(memory));
    for (i = 0; i < n; i++)
	{
        int found = 0;
        for (j= 0; j < frame_count; j++)
		{
            if (memory[j] == pages[i])
			{
                (*hits)++;
                found = 1;
                break;
            }
        }
        if (!found)
		{
            memory[front] = pages[i];
            front = (front + 1) % frame_count;
            faults++;
        }
    }
    return faults;
}
int simulate_lru(int pages[], int n, int frame_count, int *hits)
{
    int memory[frame_count];
    int last_used[frame_count];
    int faults = 0, time = 0,i,j;
    *hits = 0;
    memset(memory, -1, sizeof(memory));
    for (i = 0; i < n; i++)
	{
        int found = 0;
        for (j = 0; j < frame_count; j++)
		{
            if (memory[j] == pages[i])
			{
                (*hits)++;
                last_used[j] = time++;
                found = 1;
                break;
            }
        }
        if (!found)
		{
            int lru_index = 0;
            for (j = 1; j < frame_count; j++)
			{
                if (memory[j] == -1 || last_used[j] < last_used[lru_index])
                    lru_index = j;
            }
            memory[lru_index] = pages[i];
            last_used[lru_index] = time++;
            faults++;
        }
    }
    return faults;
}
int simulate_optimal(int pages[], int n, int frame_count, int *hits)
{
    int memory[frame_count];
    int faults = 0,i,j;
    *hits = 0;
    memset(memory, -1, sizeof(memory));
    for (i = 0; i < n; i++)
	{
        int found = 0;
        for (j = 0; j < frame_count; j++)
		{
            if (memory[j] == pages[i])
			{
                (*hits)++;
                found = 1;
                break;
            }
        }
        if (!found)
		{
            int replace_index = -1, farthest = -1;
            for (j = 0; j < frame_count; j++)
			{
                int k;
                for (k = i + 1; k < n; k++)
				{
                    if (memory[j] == pages[k]) break;
                }
                if (k == n)
				{
                    replace_index = j;
                    break;
                }
                if (k > farthest)
				{
                    farthest = k;
                    replace_index = j;
                }
            }
            memory[replace_index] = pages[i];
            faults++;
        }
    }
    return faults;
}
// ---------- Segmentation and Fragmentation ----------
Segment memory_segments[MEMORY_SIZE];
int segment_count = 0;
void initialize_memory()
{
    segment_count = 1;
    memory_segments[0].base = 0;
    memory_segments[0].size = MEMORY_SIZE;
    memory_segments[0].allocated = 0;
    memory_segments[0].process_id = -1;
}
void allocate_segment(int pid, int size)
{
	int i;
    for (i = 0; i < segment_count; i++)
	{
        if (!memory_segments[i].allocated && memory_segments[i].size >= size)
		{
            Segment new_seg = {
                .base = memory_segments[i].base,
                .size = size,
                .allocated = 1,
                .process_id = pid
            };
            Segment hole =
			{
                .base = new_seg.base + size,
                .size = memory_segments[i].size - size,
                .allocated = 0,
                .process_id = -1
            };
            memory_segments[i] = new_seg;
            if (hole.size > 0)
                memory_segments[++i] = hole;
            segment_count = i + 1;
            printf("Segment allocated to Process %d\n", pid);
            return;
        }
    }
    printf("Memory allocation failed for Process %d (Fragmentation)\n", pid);
}
void display_memory()
{
	int i;
    printf("\n-- Memory State --\n");
    for (i = 0; i < segment_count; i++)
	{
        printf("[%d - %d] %s (PID: %d)\n",
               memory_segments[i].base,
               memory_segments[i].base + memory_segments[i].size - 1,
               memory_segments[i].allocated ? "Allocated" : "Hole",
               memory_segments[i].process_id);
    }
}
void compact_memory()
{
    Segment new_memory[MEMORY_SIZE];
    int new_count = 0, current_base = 0,i;
    for (i = 0; i < segment_count; i++)
	{
        if (memory_segments[i].allocated)
		{
            new_memory[new_count] = memory_segments[i];
            new_memory[new_count].base = current_base;
            current_base += new_memory[new_count].size;
            new_count++;
        }
    }
    if (current_base < MEMORY_SIZE)
	{
        new_memory[new_count].base = current_base;
        new_memory[new_count].size = MEMORY_SIZE - current_base;
        new_memory[new_count].allocated = 0;
        new_memory[new_count].process_id = -1;
        new_count++;
    }
    memcpy(memory_segments, new_memory, sizeof(Segment) * new_count);
    segment_count = new_count;
    printf("Memory Compacted Successfully!\n");
}
// ---------- Main Menu ----------
int main()
{
    int choice,i;
    while (1)
	{
        printf("\n--- Virtual Memory Management ---\n");
        printf("1. Paging (FIFO / LRU / Optimal)\n");
        printf("2. Segmentation + Fragmentation\n");
        printf("3. Compact Memory\n");
        printf("4. Exit\nChoose an option: ");
        scanf("%d", &choice);
        if (choice == 1)
		{
            int pages[MAX], n, frames, algo, hits = 0, faults = 0;
            printf("Enter number of pages: ");
            scanf("%d", &n);
            printf("Enter page reference string: ");
            for (i = 0; i < n; i++)
                scanf("%d", &pages[i]);
            printf("Enter number of frames: ");
            scanf("%d", &frames);
            printf("Choose algorithm:\n1. FIFO\n2. LRU\n3. Optimal\nChoice: ");
            scanf("%d", &algo);
            if (algo == 1)
                faults = simulate_fifo(pages, n, frames, &hits);
            else if (algo == 2)
                faults = simulate_lru(pages, n, frames, &hits);
            else if (algo == 3)
                faults = simulate_optimal(pages, n, frames, &hits);
            else
			{
                printf("Invalid option.\n");
                continue;
            }
            printf("Page Hits: %d\n", hits);
            printf("Page Faults: %d\n", faults);
        }
		else if (choice == 2)
		{
            initialize_memory();
            int seg_choice;
            while (1)
			{
                printf("\nSegmentation Menu:\n");
                printf("1. Allocate Segment\n2. Display Memory\n3. Back\nChoice: ");
                scanf("%d", &seg_choice);
                if (seg_choice == 1)
				{
                    int pid, size;
                    printf("Enter Process ID and Segment Size: ");
                    scanf("%d %d", &pid, &size);
                    allocate_segment(pid, size);
                }
				else if (seg_choice == 2)
				{
                    display_memory();
                } else break;
            }
        }
		else if (choice == 3)
		{
            compact_memory();
            display_memory();
        }
		else if (choice == 4)
		{
            printf("Exiting...\n");
            break;
        }
		else
		{
            printf("Invalid option!\n");
        }
    }
    return 0;
}
