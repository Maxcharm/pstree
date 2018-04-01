#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctype.h>

char process[1000][20];
typedef struct Proc{
   int Pid, PPid;
   int visited, hideous, rec;
   char name[40];
}Proc;
Proc Hideous[1024];
Proc Process[1024];

int header, NUM_proc, NUM_total, NUM_hideous;
int first_construct, p;

int count_process(){
  char process[1024][40];
  NUM_proc = 0;
  DIR *directory_pointer = opendir("/proc/");
    struct dirent *entry;
      while((entry = readdir(directory_pointer)) != NULL){
	if(entry->d_name[0] >= '0' && entry->d_name[0] <= '9'){
	  strcpy(process[NUM_proc++], entry->d_name);
	}
      }
  closedir(directory_pointer);
  for(int i = 0; i < NUM_proc; i++){
    char PATH[256];
    int process_number = atoi(process[i]);
    if(process_number != 0){
       Process[i].Pid = process_number;
    }    
    strcpy(PATH, "/proc/");
    strcat(PATH, process[i]);
    strcat(PATH, "/status");

    FILE *file_pointer = fopen(PATH, "r");
    char buf[1024];
    while(!feof(file_pointer)){
       fgets(buf, 1024, file_pointer);
       int size = strlen(buf);
       if(!strncmp(buf, "Name", 4)){
         int length = 0;
	 for(int k = 4; k < size; k++){
	   if(buf[k] >= 'a' && buf[k] <= 'z'){
	     Process[i].name[length] = buf[k];
	     length++; 
	   }
	 }
       }
       else if(!strncmp(buf, "PPid", 4)){
           int length = 0;
	   char temp[40];
	   for(int k = 4; k < size; k++){
	     if(buf[k] >= '0' && buf[k] <= '9'){
	       temp[length] = buf[k];
	       length++;
	     }
	   }
	   temp[length] = '\0';
	   Process[i].PPid = atoi(temp);
       }
    }
    fclose(file_pointer);
  }
  return NUM_proc;
}

int hideous_counting(int NUM_proc){
   NUM_hideous = 0;
   char hide[1000][20];
   DIR *entry = opendir("/proc");
   struct dirent *directory_pointer1, *directory_pointer2;
   while((directory_pointer1 = readdir(entry)) != NULL){
       if(directory_pointer1->d_name[0] >= '0' && directory_pointer1->d_name[0] <= '9'){
          char PATH[1024];
	  strcpy(PATH, "/proc/");
	  strcat(PATH, directory_pointer1->d_name);
	  strcat(PATH, "/task");
	  DIR *entry2 = opendir(PATH);
	  while((directory_pointer2 =readdir(entry2)) != NULL){
	    if(directory_pointer2->d_name[0] >= '0' && directory_pointer2->d_name[0] <= '9'){
	       if(strcmp(directory_pointer1->d_name, directory_pointer2->d_name) != 0){
	            strcpy(hide[NUM_hideous], directory_pointer2->d_name);
		    char path[1024];
		    if(atoi(hide[NUM_hideous]) != 0){
		        Process[NUM_hideous + NUM_proc].Pid = atoi(hide[NUM_hideous]);
			Process[NUM_hideous + NUM_proc].hideous = 1;
		    } 
		    strcpy(path, PATH);
		    strcat(path, hide[NUM_hideous]);
		    strcat(path, "/status");
		    FILE *file_pointer = fopen(path, "r");
		    char buf[1024];
		    if(file_pointer){
			for(int tms = 0; tms < 8; tms++){
		        fgets(buf, 1024, file_pointer);
			int size = strlen(buf);
			if(!strncmp(buf, "Tgid", 4)){
			    int length =  0;
			    char temp[40];
			    for(int i = 4; i < size; i++){
			        if(buf[i] >= '0' && buf[i] <= '9'){
				   temp[length] = buf[i];
				   length++;
				}	
			    }
			    temp[length] = '\0';
			    if(atoi(temp) != 0){
			      Process[NUM_proc + NUM_hideous].PPid = atoi(temp);
                            }
			}
			   else if(!strncmp(buf, "Name", 4)){
			      int length = 0;
			      for(int i = 4; i < size; i++){
			        if(buf[i] >= 'a' && buf[i] <= 'z'){
				  Process[NUM_proc + NUM_hideous].name[length] = buf[i];
				  length++;
				}
			      }
			      Process[NUM_proc + NUM_hideous].name[length] = '\0';
			   } 
			}
		        fclose(file_pointer);

		    }
		    else continue;
	       NUM_hideous++;
	       }
	    }
	  }
	  closedir(entry2);
       }
   }
   closedir(entry);
   return NUM_hideous;
}

void tree_constructing(int PPid, int rec){
    if(first_construct == 1 && header !=0){
       for(int i = 0; i < NUM_total; i++){
         if(Process[i].Pid == PPid){
            printf("├─%s", Process[i].name);
	    if(p){
	      printf("(%d)", Process[i].Pid);
	    }
	    printf("\n");
	    first_construct = 0;
	    rec++;
	    break;
	 }
       }
    }
    for(int i = 0; i < NUM_total; i++){
       if(Process[i].PPid == PPid && (!Process[i].visited)){
            Process[i].rec = rec + 1;
	    for(int k = 0; k < rec; k++){
	       printf("│          ");
	    }
	    Process[i].visited = 1;
	    if(Process[i].Pid == 0){
	       continue;
	    }
	    char father_of_all[9] = "systemd";
	    if(Process[i].Pid == 1){
	       strcpy(Process[i].name, father_of_all);
	    }
	    if(Process[i].hideous){
	      printf("├─{%s}", Process[i].name);
	      if(p){
	         printf("(%d)", Process[i].Pid);
	      }
	      printf("\n");
	    }
	    else{
	       printf("├─%s", Process[i].name);
	       if(p){
	         printf("(%d)", Process[i].Pid);
	       }
	       printf("\n");
	    }
	    tree_constructing(Process[i].Pid, Process[i].rec);

       }
    }    
}

int main(int argc, char *argv[]) {
    NUM_proc = count_process();
    NUM_hideous = hideous_counting(NUM_proc);
    NUM_total = NUM_proc + NUM_hideous;
    //printf("%d %d %d", NUM_proc, NUM_hideous, NUM_total);
    //printf("%d\n", argc);
    p = 0;
    header = 0;
    memset(&Process->rec, 0, NUM_total);
    memset(&Process->visited, 0, NUM_total);
    first_construct = 1;
    char prefix[] = "./pstree";
    char fun1[] = "-p";
    char fun11[] = "--show-pids";
    char fun2[] = "-n";
    char fun22[] = "--numeric-sort";
    char fun3[] = "-V";
    char fun33[] = "--version";
    for(int i = 0; i < argc; i++){
       assert(argv[i]);
    }
    assert(!argv[argc]);
    if(strcmp(argv[0], prefix) != 0){
      exit(0);
    }
    if(argc == 2){
       if((!strcmp(argv[1], fun3)) || (!strcmp(argv[1], fun33))){
          printf("pstree  1.0\nCopyright (C) 2018-8102 by Maxcharm Han\nMaxcharm comes with ABSOLUTELY NO WARRANTY.\nAnd this is free software.\n");
	 exit(0); 
       }
       else if((!strcmp(argv[1], fun1)) || (!strcmp(argv[1],fun11))){
         p = 1;
       }
       else if((!strcmp(argv[1], fun2)) || (!strcmp(argv[1],fun22))){
         p = 0;
	 Proc temp;
	 for(int i = 0; i < NUM_total; i++){
            for(int j = 0; j < NUM_total; j++){
                if(Process[i].Pid > Process[j].Pid){
                    temp.Pid = Process[i].Pid;
                    temp.PPid = Process[i].PPid;
		    temp.visited = Process[i].visited;
                    temp.rec = Process[i].rec;
		    temp.hideous = Process[i].hideous;
		    strcpy(temp.name, Process[i].name);
		    Process[i].Pid = Process[j].Pid;
		    Process[i].PPid = Process[j].PPid;
		    Process[i].rec = Process[j].rec;
		    Process[i].visited = Process[j].visited;
		    Process[i].hideous = Process[j].hideous;
		    strcpy(Process[i].name, Process[j].name);
		    Process[j].Pid = temp.Pid;
		    Process[j].PPid = temp.PPid;
		    Process[j].rec = temp.rec;
		    Process[j].visited = temp.visited;
		    Process[j].hideous = Process[j].hideous;
		    strcpy(Process[j].name, temp.name);
		}
	    }
	 }
      }
      else{
        printf("Invalid input.\n");
	exit(0);
      }
      tree_constructing(header, 0);
    }
    if(argc == 3){
       if(!strcmp(argv[1], fun1) || !strcmp(argv[1], fun11)){
         p = 1;
       }
       if(!strcmp(argv[1], fun2) || !strcmp(argv[1], fun22)){
        Proc temp;
   	for(int i = 0; i < NUM_total; i++){
	  for(int j = 0; j < NUM_total; j++){
	    if(Process[i].Pid > Process[j].Pid){
		temp.Pid = Process[i].Pid;
		temp.PPid = Process[i].PPid;
		temp.visited = Process[i].visited;
		temp.rec = Process[i].rec;
		temp.hideous = Process[i].hideous;
		strcpy(temp.name, Process[i].name);
		Process[i].Pid = Process[j].Pid;
		Process[i].PPid = Process[j].PPid;
		Process[i].rec = Process[j].rec;
		Process[i].visited = Process[j].visited;
		Process[i].hideous = Process[j].hideous;
		strcpy(Process[i].name, Process[j].name);
		Process[j].Pid = temp.Pid;
		Process[j].PPid = temp.PPid;
		Process[j].rec = temp.rec;
		Process[j].visited = temp.visited;
		Process[j].hideous = Process[j].hideous;
		strcpy(Process[j].name, temp.name);
	    }
	  }
	}
    		p = 0;
       }
       int length = strlen(argv[2]);
       for(int i = 0; i < length; i++){
           if(argv[2][i] < '0' || argv[2][i] > '9'){
	      printf("Invalid pid. Only numbers are acceptable.\n");
	   }
       }
       int pid_temp = atoi(argv[2]);
       int flag1 = 0;
       for(int j = 0; j < NUM_total; j++){
          if(Process[j].Pid == pid_temp)
		  flag1 = 1;
       } 
       if(flag1){
         header = pid_temp;
       }
       else{
         printf("Pid not found.\n");
	 exit(0);
       }
      tree_constructing(header,0);
    } 
  return 0;  
}

