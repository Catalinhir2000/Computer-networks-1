#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>

int succes = 0;
int autorizare = 0;
void login(char* uid)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("an error ocurred with openning the pipe\n");
        exit(1);
    }
    
    
    int id = fork();
    if (id == -1)
    {
        perror("an error ocurred with the fork\n");
        exit(2);
    }
    if (id == 0)
    {
        FILE * fp;
        fp = fopen("username.txt", "r");
        if (fp == NULL)
        {
            perror("unable to open the file");
            exit(3);
        }
        char verifid[128];
        while (fgets(verifid, sizeof(verifid), fp) != NULL) // sizeof/strlen
        {
            verifid[strlen(verifid) - 1] = '\0';
            if (strcmp(uid, verifid) == 0)
            {
                succes = 1;
                close(fd[0]);
                if (write(fd[1], &succes, sizeof(int)) == -1)
                {
                    perror("an error ocurred with writing in the pipe\n");
                    exit(4);
                }
                close(fd[1]);
            }
            

        }
        fclose(fp);
        exit(0);
    }
    
    else
    {
        wait(NULL);
        int status;
        close(fd[1]);
        if (read(fd[0], &status, sizeof(int)) == -1)
        {
            perror("an error ocurred with reading from the pipe\n");
            exit(5);
        }
        close(fd[0]);
        if (status == 1)
        {
            printf("AUTENTIFICARE CU SUCCES, ACUM PUTETI BENEFICIA SI DE COMENZILE <<myfind>> si <<mystat>>\n");
            autorizare = 1;
        }
        else
        {
            printf("AUTENTIFICARE ESUATA\n");
        }
         
        
        

    }
    
}
char* recursivefind(char cale[4096], char nume[4096])
{
    struct dirent *sd;
        DIR *dir;
        dir = opendir(cale);
        
        /* if (dir == NULL)
        {
            perror("error with openning the directory");
            exit(12);
        } */
        
        if (dir != NULL)
        {
            while((sd = readdir(dir)) != NULL)
            {
                char scale[4096];
                strcpy(scale, cale);
                strcat(scale, "/");
                strcat(scale, sd->d_name);
                
                if (strcmp(nume, sd->d_name) == 0)
                {
                   char* calegasita = (char*) malloc(4096);
                   strcpy(calegasita, scale);
                   return calegasita; 
                }
                
                recursivefind(scale, nume);
            }
            closedir(dir);
            
        }
    

}
void myfind(char nume[4096])
{
    char path[4096];
    if(mkfifo("myfifo4", 0777) == -1)
    {
        if(errno != EEXIST)
        {
            perror("could not create the fifo file\n");
            exit(10);
        }
    }
    int id = fork();
    if (id == -1)
    {
        perror("error with the fork");
        exit(11);
    }
    if (id == 0)
    {
        int fd = open("myfifo4", O_WRONLY);
        if (fd == -1)
        {
            perror("could not open the fifo file");
            exit(12);
        }
        strcpy(path, recursivefind("/home/catalin", nume));
        if (write(fd, &path, sizeof(path)) == -1)
        {
            perror("could not write in the fifo file\n");
            exit(13);
        }
        close(fd);
        exit(0);
        
        
    }
    else
    {
        char pathy[4096];
        int fd = open("myfifo4", O_RDONLY);
        if (fd == -1)
        {
            perror("error openning the fifo file\n");
            exit(14);
        }
        if (read(fd, &pathy, sizeof(pathy)) == -1)
        {
            perror("error reading from the fifo file\n");
            exit(15);
        }
        printf("recived %s\n", pathy);
        close(fd);
        
        
    }
    
    

}
void mystat(char calea[1024])
{
    int sockp[2];
    char msg[1024];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
      { 
        perror("Err... socketpair"); 
        exit(16); 
      }
    int id = fork();
    if (id == -1)
    {
        perror("error with the fork");
        exit(17);
    }
    if (id == 0)
    {
        close(sockp[0]);
        struct stat fisier;
        stat(calea, &fisier);
        if (S_ISDIR(fisier.st_mode))
        {
            strcpy(msg,"director");
            if (write(sockp[1], msg, sizeof(msg)) == -1)
            {
                perror("error writing in the socketpair\n");
                exit(18);
            }
            
        }
        if (S_ISREG(fisier.st_mode))
        {
            strcpy(msg,"regular file");
            if (write(sockp[1], msg, sizeof(msg)) == -1)
            {
                perror("error writing in the socketpair\n");
                exit(19);
            }
        }
        if (S_ISCHR(fisier.st_mode))
        {
            strcpy(msg,"character device");
            if (write(sockp[1], msg, sizeof(msg)) == -1)
            {
                perror("error writing in the socketpair\n");
                exit(20);
            }
        }
        if (S_ISLNK(fisier.st_mode))
        {
            strcpy(msg,"symbolic link");
            if (write(sockp[1], msg, sizeof(msg)) == -1)
            {
                perror("error writing in the socketpair\n");
                exit(21);
            }
        }
        if (S_ISSOCK(fisier.st_mode))
        {   
            strcpy(msg,"socket");
            if (write(sockp[1], msg, sizeof(msg)) == -1)
            {
                perror("error writing in the socketpair\n");
                exit(22);
            }
        }
        if (S_ISBLK(fisier.st_mode))
        {
            strcpy(msg,"block device");
            if (write(sockp[1], msg, sizeof(msg)) == -1)
            {
                perror("error writing in the socketpair\n");
                exit(23);
            }
        }
        if (S_ISFIFO(fisier.st_mode))
        {   
            strcpy(msg,"FIFO file");
            if (write(sockp[1], msg, sizeof(msg)) == -1)
            {
                perror("error writing in the socketpair\n");
                exit(24);
            }
        }
        close(sockp[1]);
        exit(0);
    }
    else
    {
        char message[1024];
        close(sockp[1]);
        if (read(sockp[0], message, sizeof(msg)) == -1)
        {
            perror("error with reading from the socketpair\n");
            exit(25);
        }
        printf("%s\n", message);
        close(sockp[0]);
    }
    
    
    
}
/* void quit()
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("an error ocurred with openning the pipe\n");
        exit(6);
    }
    
    int id = fork();
    if (id == -1)
    {
        perror("an error ocurred with the fork\n");
        exit(7);
    }
    
    if (id == 0)
    {
        close(fd[0]);
        int child = getpid();
        int parent = getppid();
        if (write(fd[1], &parent, sizeof(int)) == -1)
        {
            perror("an error occured with writing to the pipe\n");
            exit(8);
        }
        close(fd[1]);
        kill(child, SIGTERM);
        
    } 
     else
    {
        wait(NULL);
        close(fd[1]);
        int parentid;
        if (read(fd[0], &parentid, sizeof(parentid)) == -1)
        {
            perror("an error ocurred with reading from the pipe\n");
            exit(9);
        }
        close(fd[0]);
        kill(parentid, SIGTERM);
        
    } 
    
}  */


int main (int argc, char* argv[])
{
    char calea[1024];
    char nume[128];
    char userid[1024];
    printf("COMENZI DISPONIBILE:\n");
    printf("login\n");
    printf("quit\n");
    printf("PENTRU A BENEFICIA DE COMENZILE <<myfind>> SI <<mystat>> TREBUIE SA VA CONECTATI CU UN CONT DE UTILIZATOR\n");
    int program = 1;
    while (program) //programul se repeta pana la quit.
    {
        int comandacorecta = 0;
        char command[1024];
            scanf("%s", command);

        if (strcmp(command, "login") == 0)
        {
            if (autorizare == 1)
            {
                printf("SUNTETI DEJA LOGAT CA %s", userid);
            }
            if (autorizare == 0)
            {
                
                printf("NUME DE UTILIZATOR:\n");
                scanf("%s", userid);
                login(userid);
                
            }
            if (autorizare == 0)
            {
                printf("COMENZI DISPONIBILE:\n");
                printf("login\n");
                printf("quit\n");
                printf("PENTRU A BENEFICIA DE COMENZILE <<myfind>> SI <<mystat>> TREBUIE SA VA CONECTATI CU UN CONT DE UTILIZATOR\n");
            }
            
            
            
            
            comandacorecta = 1;
            
        }
        
        
        
        if (strcmp(command, "myfind") == 0)
        {
            if (autorizare == 1)
            {
                printf("SCRIETI FISIERUL PE CARE DORITI SA IL CAUTATI\n");
                scanf("%s", nume);
                myfind(nume);
            }
            else
            {
                printf("NU AVETI ACCES LA ACEASTA COMANDA\n");
            }
            comandacorecta = 1;
            
            
        }
        if (strcmp(command, "mystat") == 0)
        {
            if(autorizare == 1)
            {
                printf("SCRIETI CALEA FISIERULUI DESPRE CARE DORITI MAI MULTE INFORMATII\n");
                scanf("%s", calea);
                mystat(calea);
            }
            else
            {
                printf("NU AVETI ACCES LA ACEASTA COMANDA\n");
            }
            comandacorecta = 1;
        } 

        if (strcmp(command, "quit") == 0)
        {
            //quit();
            comandacorecta = 1;
            exit(0);  
        }

        if (comandacorecta == 0)
        {
            printf("COMANDA INTRODUSA ESTE GERSITA!");
        }
        
    
    
    

        
    }
    return 0;
}