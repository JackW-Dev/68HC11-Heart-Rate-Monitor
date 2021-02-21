#include <stdio.h>

/*Prototypes*/
void monitorPulse();
unsigned char myKeyScan();
int login();
int isValidCredentials(char[], char[]);
void viewDataLog();
int adjustBounds();
int doctorMenu();
int nurseMenu();
int isWhitespace(char);
/*####*/

/*Timer Declarations*/
unsigned char displayFlag = 0, seconds = 0, minutes = 0, hours = 0, *timerFlag1, *toc1;
unsigned int *timerCountRegister;
int ticks = 0;
/*####*/

/*Patient Data (Logged Details)*/
char patientID[8] = "";
int accessLevel = 0, minBound = 30, maxBound = 120, minAlarmTriggers = 0, maxAlarmTriggers = 0, maxBPM = 0, minBPM = 0;
/*####*/

void main(void){
	/* Author: Mr Jack Walker
	Functions used: Scanf(), Printf()
	Purpose:
		To bring together the other functions and build a working heart rate monitor.
		Uses login() to return a user access level and redirects accordingly (Doctor can adjuct BPM safe bounds, Nurse can not)
		Doctor may monitor pulse, adjust safety bounds, view data log or exit
		Nurse may monitor pulse, view data log or exit
		System redirects accordingly
	Version: 1.0
	Modifications: None
	*/
	
	int choice = 0;
	
	for(;;){	
		printf("Login\n\r");
		accessLevel = login();
		
		while(accessLevel != 0){
			switch(accessLevel){
				case 1:
					printf("Logged in as doctor\n\r");
					choice = doctorMenu();
					
					switch(choice){
						case 1:
							/*Ask for patient details & begin reading pulse*/
							printf("Please input patient ID (max length 8)\n\rPatient ID: ");
							scanf("%s", patientID);
							printf("Patient ID is %s\n\r", patientID);
									
							printf("Press any key to exit\n\r");
							/*Reset time variables to 0 (they will have been incrementing due to the interrupt)*/
							seconds = 0;
							minutes = 0;
							hours = 0;
							monitorPulse();								
							break;
						case 2:
							/*Adjust minBound & maxBound*/
							adjustBounds();
							break;
						case 3:
							/*View data log*/
							viewDataLog();
							break;
						case 4:
							/*Log out*/
							accessLevel = 0;
							break;
					}
					break;
				case 2:
					printf("logged in as nurse\n\r");
						choice = nurseMenu();
						
						switch(choice){
							case 1:
								/*Ask for patient details & begin reading pulse*/
								printf("Please input patient ID (max length 8)\n\rPatient ID: ");
								scanf("%s", patientID);
								printf("%s\n\r", patientID);
								
								printf("Press any key to exit\n\r");
								/*Reset time variables to 0 (they will have been incrementing due to the interrupt)*/
								seconds = 0;
								minutes = 0;
								hours = 0;
								monitorPulse();								
								break;
							case 2:
								/*View data log*/
								viewDataLog();
								break;
							case 3:
								/*Log out*/
								accessLevel = 0;
								break;
						}
					break;			
			}
		}
		if(accessLevel == -1){
			/*Loop to halt the system if user fails to log in after 5 attempts*/
			printf("Log in attempts exceeded, system halted. Contact IT\n\r");
			for(;;);
		}
	}
}

void monitorPulse(){
	/* Author: Mr Jack Walker
	Functions used: Printf()
	Purpose:
		Function used to detect pulses on PortA0
		Uses global variables for time in order to calculate the BPM from the pulses
		Triggers alarm if BPM exceeds minBound or maxBound
		Outputs the average BPM over 15 seconds and current time
	Version: 1.0
	Modifications: None
	*/
	
	/*Timer Declarations*/
	unsigned char *timerMask1;
	/*####*/
	
	/*PortA Declarations*/
	unsigned char *portA, *ddrA;
	/*####*/
	
	/*Pulse Declarations*/
	int i = 0, pulseCount = 0, avgBPM = 0, minFlag = 0, maxFlag = 0, BPM[3] = {0, 0, 0};
	/*####*/
	
	/*General Declarations*/
	char exitChar = 0;
	/*####*/
	
	/*Timer Initialisation*/	
	timerCountRegister = (unsigned int*)0x0e;
	timerMask1 = (unsigned char*)0x22;
	timerFlag1 = (unsigned char*)0x23;
	toc1 = (unsigned char*)0x16;	
	*timerFlag1 = 0x80;		/*1000 0000 is 0x80 in hex - this is the toc1 interrupt bit*/
	*toc1 = *timerCountRegister + 20000;
	*timerMask1 = 0x80;
	/*####*/
		
	/*PortA Initialisation*/
	portA = (unsigned char*)0x00;
	ddrA = (unsigned char*)0x01;
	*ddrA = 0x0E;
	/*####*/
	
	
	*portA = 0x00;
	
	/*Main Code*/	
	while(exitChar == 0){
		while((*portA & 0x01) == 0);
		
		*portA = *portA | 0x02;	/*Turn on portA1*/
			
		while((*portA & 0x01) == 1);
		
		*portA = *portA ^ 0x02;	/*Turn off portA1*/
		
		pulseCount++;
		/*printf("Data = %d index = %d BPM[0] = %d BMP[1] = %d BMP[2] = %d Seconds %% 5 = %d\n\r", pulseCount, i, BPM[0], BPM[1], BPM[2], seconds%5);*/
		
		exitChar = myKeyScan();
		
		if(displayFlag){
			if(pulseCount > 8){
				pulseCount--;	/*Found from running code multiple times that if over 8 beats are detected, anomalies occur. Reducing pulseCount by 1 fixed this*/
			}
			
			/*Log lowest or highest BPM*/
			BPM[i] = (pulseCount - 1) * 12;
			if(BPM[i] < minBPM){
				minBPM = BPM[i];
			}else if(BPM[i] > maxBPM){
				maxBPM = BPM[i];
			}
			/*####*/
			
			if(BPM[i] < minBound){
				*portA = *portA | 0x04;
				printf("Recent pulse is below minimum bound (%d)\n\r", minBound);
				minFlag = 1;
				minAlarmTriggers++;
			}else if(BPM[i] > maxBound){
				*portA = *portA | 0x08;
				printf("Recent pulse is above maximum bound (%d)\n\r", maxBound);
				maxFlag = 1;	
				maxAlarmTriggers++;			
			}
			else{
				if(minFlag == 1){
					*portA = *portA ^ 0x04;
					minFlag = 0;
				}else if(maxFlag == 1){
					*portA = *portA ^ 0x08;
					maxFlag = 0;
				}
			}
			
			exitChar = myKeyScan();
			
			if(seconds == 5 && (BPM[1] == 0 && BPM[2] == 0)){
				BPM[1] = (pulseCount - 1) * 12; 
				BPM [2] = (pulseCount - 1) * 12;
			}
			else if(seconds == 10 && BPM[2] == 0){
				BPM[2] = (pulseCount - 1) * 12;
			}
			
			avgBPM = (BPM[0] + BPM[1] + BPM[2])/3;
			printf("%02d:%02d:%02d\tBPM = %d\n\r", hours, minutes, seconds, avgBPM);
			
			pulseCount = 0;
			displayFlag = 0;
				
			i++;			
			if(i == 3){
				i = 0;
			}
			exitChar = myKeyScan();	
		}
	}
	printf("Stopping pulse read.\n\t");
	/*####*/
}

/*Timer*/
@interrupt void timer(void){
	/* Author: Mr Jack Walker
	Functions used: None
	Purpose: System interruptm using TOC in order to produce an accurate system clock
	Version: 1.0
	Modifications: None
	*/		
	timerFlag1 = (unsigned char*)0x23;
	toc1 = (unsigned char*)0x16;
	timerCountRegister = (unsigned int*)0x0e;

	ticks++;
	
	/*1 Tick is 1/100th of a second*/
	if(ticks == 100){
		seconds++;
		ticks = 0;	
	}
	if (seconds == 60){
		seconds = 0;
		minutes++;
	}
	if(minutes == 60){
		minutes = 0;
    	hours++;
	}
	if(hours == 24){
		hours = 0;
	}
	if(seconds % 5 == 0 && ticks == 0){
		displayFlag = 1;
	}
	
	*timerFlag1 = 0x80;
	*toc1 = *timerCountRegister + 20000;	
}
/*####*/

unsigned char myKeyScan(){
	/* Author Mr james Mccarren
	Company: Staffordshire University
	Functions used:None
	Purpose: Simple keyscan with no handshaking using 68hc11 microcontroller
	Version: 1.0
	Modifications: None
	*/
	unsigned char *scsr, *scdr, data;
	/*Note no config needed for baud rate since already defined in monitor program*/
	scsr = (unsigned char *)0x2e;
	scdr = (unsigned char *)0x2f;
	data = '\0';
	/*If key has been pressed read key code else return NULL */
	if (((*scsr) & 0x20) != 0x0)
	{
		data = *scdr;
	}
	return(data);
}

int login(){
	/* Author: Mr Jack Walker
	Functions used: Scanf(), Printf()
	Purpose: Login system validating user input, only allowing valid users to access the system
	Version: 1.0
	Modifications: None
	*/
	int attempt = 0, valid = 0;
		
	char doctor[8] = {'S','m','i','t','h','_','1','\0'};
	char nurse[8] = {'F','o','r','d','_','1','0','\0'};
	char doctorPass[8] = {'L','e','t','M','3','1','n','\0'};
	char nursePass[8] = {'A','c','c','3','s','_','1','\0'};
	char uName[8], pass[8];
	
	do{
		printf("Username: ");
		scanf("%s", &uName);
		
		printf("Password: ");
		scanf("%s", &pass);
		
		if((isValidCredentials(doctor, uName) == 1) && (isValidCredentials(doctorPass, pass) == 1)){
			valid = 1;
		}else if((isValidCredentials(nurse, uName) == 1) && (isValidCredentials(nursePass, pass) == 1)){
			valid = 2;
		}
		else{
			attempt++;
			printf("Invalid data given, please try again.\n\r");
		}	
	}while(attempt < 5 && valid == 0);
	if(valid != 0){
		return valid;
	}
	return 0;
}

int isValidCredentials(char a[], char b[]){
	/* Author: Mr Jack Walker
	Functions used: None
	Purpose: Simple character comparison function that returns 1 if the characters match and 0 if they do not
	Version: 1.0
	Modifications: None
	*/
	int i;
	for(i = 0; i < 8; i++){
		if(a[i] != b[i]){
			return 0;
		}
	}
	return 1;	
}

void viewDataLog(){
	/* Author: Mr Jack Walker
	Functions used: Printf()
	Purpose: Output a basic data log for the most recent patient
	Version: 1.0
	Modifications: None
	*/
	if(accessLevel == 1){
		printf("User: Doctor\n\r");
	}else{
		printf("User: Nurse\n\r");
	}
	printf("Patient ID: %s\n\r", patientID);
	printf("Minimum pulse boundary %d BPM\n\r", minBound);
	printf("Maximum pulse boundary %d BPM\n\r", maxBound);
	printf("Minimum pulse recorded %d BPM\n\r", minBPM);
	printf("Maximum pulse recorded %d BPM\n\r", maxBPM);
	printf("Mimimum alarm raised %d times\n\r", minAlarmTriggers);
	printf("Maximum alarm raised %d times\n\r", maxAlarmTriggers);
}

int adjustBounds(){  
	/* Author: Mr Jack Walker
	Functions used: Printf(), Scanf(), isWhitespace() (My own version)
	Purpose: Adjust pulse bounds, ensuring that only valid values can be input
	Version: 1.0
	Modifications: None
	*/ 
    int minBoundValid = 0, maxBoundValid = 0, valueRead;
    char followChar;
    
    do{
        printf("Minimum bound: ");
        valueRead = scanf("%d%c", &minBound, &followChar);
        if(valueRead == 2) {
            if(isWhitespace(followChar) && minBound >= 10 && minBound < 120) {
                /*Integer followed by whitespace*/
                minBoundValid = 1;
            }else{
                /*Integer followed by character*/
				printf("Value must above 10 and below 120.\n\r");
            }
        }else if(valueRead == 1 && minBound >= 10 && minBound <= 120) {
            /*Integer followed by nothing*/
            minBoundValid = 1;
        }else{
            /*Not an integer*/
            printf("Only integer values will be accepted, please try again.\n\r");
            while ((getchar()) != '\n'); 
        }
    }while(minBoundValid == 0);
    
    do{
        printf("Maximum bound: ");
        valueRead = scanf("%d%c", &maxBound, &followChar);
        if(valueRead == 2) {
            if(isWhitespace(followChar) && maxBound > minBound && maxBound <= 200) {
                /*Integer followed by whitespace*/
                maxBoundValid = 1;
            }else{
                /*Integer followed by character*/
				printf("Value must above %d and below 200.\n\r", minBound);
            }
        }else if(valueRead == 1 && maxBound > minBound && maxBound <= 200) {
            /*Integer followed by nothing*/
            maxBoundValid = 1;
        }else{
            /*Not an integer*/
            printf("Only integer values will be accepted, please try again.\n\r");
            while ((getchar()) != '\n'); 
        }
    }while(maxBoundValid == 0);
}

int doctorMenu(){
	/* Author: Mr Jack Walker
	Functions used: Printf(), Scanf()
	Purpose: Provide a validated menu system for doctors using the system
	Version: 1.0
	Modifications: None
	*/
	int valid = 0, choice = 0, valueRead;
    char followChar;
    
    do{
        printf("1 - Monitor Pulse\n\r");
		printf("2 - Alter Pulse Boundaries\n\r");
		printf("3 - View Data Log\n\r");
		printf("4 - Log Out\n\r");
        valueRead = scanf("%d%c", &choice, &followChar);
        if(valueRead == 2) {
            if(isWhitespace(followChar) && choice > 0 && choice < 5) {
                /*Integer followed by whitespace*/
                valid = 1;                
            }else{
                /*Integer followed by character*/
				printf("Please select a valid value.\n\r");
            }
        }else if(valueRead == 1 && choice > 0 && choice < 5) {
            /*Integer followed by nothing*/
            valid = 1;
        }else{
            /*Not an integer*/
            printf("Only integer values will be accepted, please try again.\n\r");
            while ((getchar()) != '\n'); 
        }
    }while(valid == 0);
    return choice;
}

int nurseMenu(){
	/* Author: Mr Jack Walker
	Functions used: Printf(), Scanf()
	Purpose: Provide a validated menu system for nurses using the system
	Version: 1.0
	Modifications: None
	*/
	int valid = 0, choice = 0, valueRead;
    char followChar;
    
    do{
        printf("1 - Monitor Pulse\n\r");
		printf("2 - View Data Log\n\r");
		printf("3 - Log Out\n\r");
        valueRead = scanf("%d%c", &choice, &followChar);
        if(valueRead == 2) {
            if(isWhitespace(followChar) && choice > 0 && choice < 4) {
                /*Integer followed by whitespace*/
                valid = 1;                
            }else{
                /*Integer followed by character*/
				printf("Please select a valid value.\n\r");
            }
        }else if(valueRead == 1 && choice > 0 && choice < 4) {
            /*Integer followed by nothing*/
            valid = 1;
        }else{
            /*Not an integer*/
            printf("Only integer values will be accepted, please try again.\n\r");
            while ((getchar()) != '\n'); 
        }
    }while(valid == 0);
    return choice;
}

int isWhitespace(char a){
	/* Author: Mr Jack Walker
	Functions used: None
	Purpose: Check if an incoming character is whitespace or not
	Version: 1.0
	Modifications: None
	*/
	switch(a){
		case ' ':	/*Space*/
			return 1;
		case '\n':	/*Newline*/
			return 1;
		case '\t':	/*Tab*/
			return 1;
		case '\v':	/*Vertical tab*/
			return 1;
		case '\f':	/*Form feed*/
			return 1;
		case '\r':	/*Carriage return*/
			return 1;
		default:
			return 0;
	}
}
