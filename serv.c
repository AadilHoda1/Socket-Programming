

// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

// function to get balance of a client
// this function calculates balance by going through all transactions in a client's account
float get_bal(FILE *f){
	float amount = 0, total=0;
	char date[25], type, c;
	fseek(f,0,SEEK_SET);
    while(fscanf(f,"%[^,],%c,%f",date,&type,&amount))
    {
         if(feof(f)){
            break;
        }
        fscanf(f,"%c",&c);
				//adding amount for credit and subtract for debit
        if(type == 'C')
            total += amount;
        else
            total -= amount;

    }
    return total;
}

//this funtion will disply 10 most recent transations done into a client's account
void get_mini_stmt(int socket, FILE *in){
	int count = 0;
    long int pos;
    char buffer[1024], s[1024];
		//pointer at the end of transaction file
    fseek(in, 0, SEEK_END);
    pos = ftell(in);
    while (pos) {
        fseek(in, --pos, SEEK_SET); /* seek from begin */
        if (fgetc(in) == '\n') {
            if (count++ == 10) break;
        }
    }
		if(count<10){
			fseek(in, 0, SEEK_SET);
		}
	//	printf("position : %ld\n", pos);
    bzero(buffer, 1024);
    int x=0;
    /* Write line by line, is faster than fputc for each char */
    while (fgets(s, sizeof(s), in) != NULL) {
    	if(x == 0){
    		strcpy(buffer, s);
    		x=1;
    		continue;
    	}
    	strcat(buffer, s);
    }
		  //provide the options again after giving out mini statement
    strcat(buffer, "Choose: 1. Check Balances 2. Mini Statement 3. EXIT\n");
	send(socket , buffer , sizeof(buffer) , 0);


}

//function to handle bank customers
int customer(int new_socket, char buffer[1024], FILE *f1){
	read(new_socket, buffer, 1024);
    int end = 0;
    if(strcmp(buffer,"1") == 0){
        float bal = get_bal(f1);
        bzero(buffer, 0);
        sprintf(buffer, "Account Balance: %f\nChoose: 1. Check Balance 2. Print Mini Statement 3. EXIT\n" , bal);
        send(new_socket , buffer , strlen(buffer) , 0);
    }
    else if(strcmp(buffer,"2") == 0){
        get_mini_stmt(new_socket,f1);
    }
		//for anything else exit
    else{
        end =1;
    }
    bzero(buffer, 1024);
    return end;
}

// function for providing service to police
int police(int new_socket, char buffer[1024], FILE* f1){
	char c;

	char fname[256];

	int end = 0;

	read(new_socket, buffer, 1024);
	char customer[20] = "acehkmoqvs";
	c = buffer[0];
	if(c == '3')
		end = 1;
	else{
		bzero(buffer, 1024);
		//this gives balance for all customers
		if(c == '1'){
			int i = 0;
			for( i=0;i<strlen(customer); i++){
				printf("length: %d \n", strlen(customer));
				char fn[256], s[256];
				sprintf(fn, "%c.csv", customer[i]);
				FILE *f2;
				f2 = fopen(fn, "r+");
				//get balance for all customers and print them one by one
				float bal = get_bal(f2);
				if(i==0){
					sprintf(buffer, "%c: %f\n", customer[i],bal);
					continue;
				}
				sprintf(s, "%c: %f\n", customer[i], bal);
				strcat(buffer, s);
			}
			// providing options again
			strcat(buffer, "Choose: 1. Print All Balances 2. Print Mini Statement 3. EXIT\n");
			send(new_socket , buffer , strlen(buffer) , 0);
		}
		// for checking mini statement of a customer
		else if(c == '2'){
			strcpy(buffer, "CustomerID: ");
			send(new_socket , buffer , strlen(buffer) , 0);
			bzero(buffer, 1024);
			//read client id from buffer
			read(new_socket, buffer, 1024);
			//open file of teansactions of that client
			sprintf(fname, "%s.csv", buffer);
			f1 = fopen(fname, "r+");
			//in case transaction file not presentt give error
			if(!f1){
				bzero(buffer, 1024);
				strcpy(buffer, "User not found\nChoose: 1. Print All Balances 2. Print Mini Statement 3. EXIT\n");
				send(new_socket , buffer , strlen(buffer) , 0);
				bzero(buffer, 1024);
				return 0;
			}
			// else get mini statement of that client
			get_mini_stmt(new_socket,f1);
		}
	}
	bzero(buffer, 1024);
	return end;
}

// for handling admin functionalities
int admin(int new_socket, char buffer[1024], FILE* f1){
	read(new_socket, buffer, 1024);
	char fname[256];
	int end = 0;
	if(strcmp(buffer,"3") == 0)
		end = 1;
	else{
		char cd = 'D';
		// credit
		if(strcmp(buffer,"1") == 0)
			cd = 'C';
		bzero(buffer, 1024);
		strcpy(buffer, "CustomerID: ");
		send(new_socket , buffer , strlen(buffer) , 0);
		bzero(buffer, 1024);
		//take customer id
		read(new_socket, buffer, 1024);
		sprintf(fname, "%s.csv", buffer);
		f1 = fopen(fname, "r+");
		//check if cusotmer exists
		if(!f1){
			bzero(buffer, 1024);
			strcpy(buffer, "User not found\nChoose: 1. Credit 2. Debit 3. EXIT\n");
			send(new_socket , buffer , strlen(buffer) , 0);
			bzero(buffer, 1024);
			return 0;
		}
		//if customer exists then ask for amount to credit
		bzero(buffer, 1024);
		strcpy(buffer, "Amount: ");
		send(new_socket , buffer , strlen(buffer) , 0);
		bzero(buffer, 1024);
		read(new_socket, buffer, 1024);
		if(cd == 'D'){
			float balance = get_bal(f1);
			// get total balance of customer and give error if total balcane is less than the amount to be debited
			if(atof(buffer) > balance){
				bzero(buffer, 1024);
				strcpy(buffer, "Insufficient Balance\nChoose: 1. Credit 2. Debit 3. EXIT\n");
				send(new_socket , buffer , strlen(buffer) , 0);
				bzero(buffer, 1024);
				return 0;
			}
		}
		//to put time and amount and D (for debit) or C (for credit) in clients transaction file
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		printf("New Transaction at: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        fseek(f1,0,SEEK_END);
        char p[256];
        sprintf(p, "%d-%d-%d %d:%d:%d,%c,%s", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, cd, buffer);

        fprintf(f1,"%s\n",p);
        fflush(f1);
        fflush(stdin);
        bzero(buffer, 1024);
				// if succesful then put transaction completed message into buffer and provide further options
        strcpy(buffer, "Transaction Completed\nChoose: 1. Credit 2. Debit 3. EXIT\n");
        send(new_socket , buffer , strlen(buffer) , 0);
	}
	bzero(buffer, 1024);
	return end;
}



int main(int argc, char const *argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char hello[1024] = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
												&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( atoi(argv[1]) );

	// Forcefully attaching socket to the port no. given by terminal command.
	if (bind(server_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	// listen from socket
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	// arrays for u_name and client password for matching
	char username[256], password[256], type;
	int end=0, n=0;
	while(1){
		if(end==0){
			if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}
			bzero(username, 256);
			bzero(password, 256);
			read( new_socket , buffer, 1024);
			printf("%s\nHello message sent\n",buffer );
			bzero(buffer,1024);

			//ask for username to login and read it from client buffer
			strcpy(buffer,"Username: ");
			send(new_socket , buffer , strlen(buffer) , 0);
			bzero(buffer,1024);
			if (n = read( new_socket , username, 256) <= 0){
				close(new_socket);
				break;
			}
			bzero(buffer,1024);
				//empty the buffer and ask for password
			strcpy(buffer,"Password: ");
			send(new_socket , buffer , strlen(buffer) , 0);
			bzero(buffer,1024);
			if (n = read( new_socket , buffer, 256) <= 0){
				close(new_socket);
				break;
			}
			strcpy(password, buffer);
			//search for the client in clients.csv file which contains name password and type of each client
			FILE *f,*f1;
			char fname[256];
			f = fopen("clients.csv","r");
			char user[256], pass[256], c;
			int t=0;
			fseek(f,0,SEEK_SET);  // fseek: positioning file pointer, seek_set: beginning of file
            while(fscanf(f,"%[^,],%[^,],%c",user,pass,&type)){
                fscanf(f,"%c",&c);
				bzero(buffer,1024);
					//search until end of file and if not found raise an error
                if (feof(f)){
                	strcpy(buffer, "Invalid username or password\n");
					send(new_socket , buffer , strlen(buffer) , 0);
					bzero(buffer, 1024);
					t = 1;
					break;
                }
								//if client found then provide further options
                if(strcmp(user, username) == 0 && strcmp(pass, password) == 0){
                	printf("User Authenticated\n");
                	if(type == 'C'){	//customer
                        strcpy(buffer,"Choose: 1. Check Balance 2. Print Mini Statement 3. EXIT\n");
						send(new_socket , buffer , strlen(buffer) , 0);
						bzero(buffer,1024);

                	}
                	else if(type == 'A'){  //admin
                        strcpy(buffer,"Choose: 1. Credit 2. Debit 3. EXIT\n");
						send(new_socket , buffer , strlen(buffer) , 0);
                		bzero(buffer,1024);
                	}
                	else{	//police
                        strcpy(buffer,"Choose: 1. Print All Balances 2. Print Mini Statement 3. EXIT\n");
						send(new_socket , buffer , strlen(buffer) , 0);
        				bzero(buffer,1024);
                	}
                	break;
                }

			}
			if(t){
				end = 1;
				sleep(0.5);
				continue;
			}

			if(type == 'C'){
				sprintf(fname, "%s.csv", username);
				f1 = fopen(fname, "r+");
				if(!f1){
					bzero(buffer, 1024);
					strcpy(buffer, "User file not found\nChoose: 1. Check Balances 2. Mini Statement 3. EXIT\n");
					send(new_socket , buffer , strlen(buffer) , 0);
					bzero(buffer, 1024);
					end = 1;
				}
				// calls the function to handle bank customers
                while(end == 0){
                    end = customer(new_socket, buffer, f1);
                }
			}
			else if(type == 'P'){
				//it calls police function written explicitly
				end = police(new_socket, buffer, f1);
				while(end == 0){
                    end = police(new_socket, buffer, f1);
                }
			}
			//for admin.. function writted explicitly
			else{
				while(end == 0){
					end = admin(new_socket, buffer, f1);
				}

			}
		}
		//end the process
		else{
			strcpy(buffer, "END");
			send(new_socket , buffer , strlen(buffer) , 0);
			printf("Connection Terminated\n\n");
			end = 0;

		}
	}
	return 0;
}
