#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <stdint.h>


/*checksum calculator*/
unsigned char checkSum(unsigned char arr[], int len) {
    unsigned char sum = 0;
    unsigned char twocompl; 
    for(int i = 0; i < (len-1); ++i) {
      sum += arr[i];
    }
    twocompl = ~sum + 1;
    return twocompl;
}

/*aqi*/
int AQI(float average){
    int aqi;
    if((average >= 0) && (average <12)){
        aqi =  0 +(average-0) * 50/12;
    }
    else if((average >= 12) && (average < 35.5)){
        aqi = 50 +(average-12) * 50/23.5;
    }
    else if((average >= 35.5) && (average < 55.5)){
        aqi = 100 +(average-35.5) * 50/20;
    }
    else if((average >= 55.5) && (average <150.5)){
        aqi = 150 +(average-55.5)  * 50/95.5;
    }
    else if((average >= 150.5) && (average < 250.5)){
        aqi = 200 +(average-150.5) * 100/100;
    }
    else if((average >= 250.5) && (average < 350.5)){
        aqi = 300 +(average-250.5) * 100/100;
    }
    else if((average >= 350.5) && (average < 550.5)){
        aqi = 400 +(average-350.5) * 100/200;
    }
    return aqi;
}


int checkFilename(char argv[]){
    if(strstr(argv,".csv")!=NULL){
        return 1;
    }
    if(strstr(argv,".dat")!=NULL){
        return 2;
    }
    return 0;
}

int prio;
int checkParams(char argv[]){
    int collumn = 0;
    int ch = 0;
    int chid = 0;
    int chtime = 0;
    int chvalues = 0;
    char *token;
    token = strtok(argv,",");
    while(token != NULL){
        if(strcmp(token,"id") == 0){
            ch ++;
            chid ++;
        }
        if(strcmp(token,"time") == 0){
            ch ++;
            chtime ++;
        }
        if(strcmp(token,"values") == 0){
            ch ++;
            chvalues ++;
        }
        if(collumn == 0){
            if(strcmp(token,"id") == 0){
                prio = 1;
            }
            if(strcmp(token,"time") == 0){
                prio = 4;
            }
            if(strcmp(token,"values") == 0){
                prio = 7;
            }
        }

        if(collumn == 1){
            if(strcmp(token,"id") == 0){
                prio = prio + 0;
            }
            if(strcmp(token,"time") == 0){
                prio = prio + 1;
            }
            if(strcmp(token,"values") == 0){
                prio = prio + 2;
            }
        }
        token = strtok(NULL,",");
        collumn ++;
    }
    if(ch != collumn || chid > 1 || chtime > 1 || chvalues > 1){return 0;}
    return collumn;
}

int checkArgument(int argc,char *argv[],int checkParam){
   
    /*check = 1: csv -> dat
    *2: dat -> csv
    *4,5: sorting no params (-asc,-dsc)
    *6,7: sorting have params (-asc,-dsc)
    */

    if(argc == 3){
        if((checkFilename(argv[1]) == 1) && (checkFilename(argv[2]) == 2)){
            return 1;
        }
        if((checkFilename(argv[1]) == 2) && (checkFilename(argv[2])) == 1){
            return 2;
        }
    }
    if(argc == 5){
        if((checkFilename(argv[1]) == 2) && (checkFilename(argv[2])) == 1 && (strcmp (argv[3],"-s") == 0)){
            if(strcmp(argv[4],"-asc")==0){
                return 4;
            }
            if(strcmp(argv[4],"-dsc")==0){
                return 5;
            }
        }
    }
    if(argc == 6){  
        if((checkFilename(argv[1]) == 2) && (checkFilename(argv[2])) == 1 && (strcmp (argv[3],"-s") == 0) && (checkParam > 0) && (checkParam < 4)){
            if(strcmp(argv[5],"-asc")==0){
                return 6;
            }
            if(strcmp(argv[5],"-dsc")==0){
                return 7;
            }
        }
    }
    return 0;
}

/*================================================CSV TO DAT======================================================*/

/*convert from csv file to dat file*/
int csv2dat(int argc, char *argv[],FILE* fpt_log_error,FILE* fpt_log_run)
{   

        /*file dust_aqi*/
        FILE* fpt_aqi = fopen(argv[1],"r");;

            if(fpt_aqi == NULL){
                printf("Error opening csv file\n");
                fprintf(fpt_log_error,"Error 02: denied access %s\n",argv[1]);
                return 1;
            }

        /*.dat file*/
        FILE* fpt_hex = fopen(argv[2],"w+");
            
            if(fpt_hex == NULL){
                printf("Error override dat file\n");
                fprintf(fpt_log_error,"Error 02: denied access %s\n",argv[2]);
                fclose(fpt_aqi);
                return 1;
            }
        
        

            char buffer[1024];
            int column;
            int row = 0;
            int row_true = 0;
            int id;
            int check = 0;
            char time1[20];
            unsigned char buffer2[12];              
            unsigned char bytearray[12];    //save byte
            unsigned char twoCompl;
            float average;
            int aqi;

           /*check time*/
            time_t t_now;
            t_now = time(&t_now);

            struct tm t_info;
            int y,m,d,h,mi,s;
            int ret;
            
            /*get data from dust_aqi*/
            while(fgets(buffer,1024,fpt_aqi)){
                char *token;
                column = 0;
                check = 0;
                row++;
                
                /*do not write "id,time,values"*/
                if(row == 1){
                    if(strcmp(buffer,"id,time,values,aqi,pollution\n")!=0){
                        printf("invalid file format\n");
                        fprintf(fpt_log_error,"Error 03: invalid file format at line %d\n",row-1);
                    }
                    continue;
                }

                token = strtok(buffer,",");
                while (token != NULL)
                {
                    /*Column 1: Id*/
                    if (column == 0) {
                        id = atoi(token);
                        if(id <= 0){
                            check = 1;   // invalid id
                        }
                    }
                    /* Column 2: Time*/
                    if (column == 1) {
                        strcpy(time1,token);
                        sscanf(time1,"%d:%d:%d %d:%d:%d",&y,&m,&d,&h,&mi,&s);
                        t_info.tm_year = y - 1900;               
                        t_info.tm_mon = m - 1;               
                        t_info.tm_mday = d;
                        t_info.tm_hour = h;
                        t_info.tm_min = mi;
                        t_info.tm_sec = s;
                        ret = mktime(&t_info);
                        if(strlen(time1)!=19 || ret == -1 || ret > t_now){
                            check = 2;   //invalid time
                        }
                                        
                    }
                    /* Column 3: Average */
                    if (column == 2) {
                        average = atof(token);
                        if(average < (float)0 || average > (float)550.5){
                            check = 3;          //invalid value
                        }
                    }
                    // Column 4: AQI
                    if (column == 3){
                        aqi = atoi(token);
                        if(aqi < 0 || abs(aqi - AQI(average))>1){            //invalid aqi
                            check=4;             
                        }
                    }

                    token = strtok(NULL, ",");
                    column++;
                }

                if(column == 5){
                    if(check == 0){
                        
                        sprintf(buffer2,"0F%02X%8X%08X%04X",id,ret,*(unsigned int*)&average,aqi);
                        char *hexstring = buffer2;
                        
                        uint8_t str_len = strlen(hexstring);

                        for (int i = 0; i < (str_len / 2); i++) {
                            sscanf(hexstring + 2*i, "%02x", &bytearray[i]);
                        }

                        twoCompl = checkSum(bytearray,13);
                        fprintf(fpt_hex,"00 %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X FF\n",bytearray[0],bytearray[1],bytearray[2],bytearray[3],bytearray[4],bytearray[5],bytearray[6],bytearray[7],bytearray[8],bytearray[9],bytearray[10],bytearray[11],twoCompl);
                        row_true++;
                    }
                    else if(check != 0) {
                        
                        fprintf(fpt_log_error,"Error 04: invalid data or data is missing at line %d\n",row-1);
                    }
                }
                else if(column!=5){
                    
                    fprintf(fpt_log_error,"Error 03: invalid file format at line %d\n",row-1);
                }
            }

            /*log*/
        fprintf(fpt_log_run,"Total number of rows: %d\n",row-1);
        fprintf(fpt_log_run,"Succesfully converted rows: %d\n",row_true);
        fprintf(fpt_log_run,"Error rows: %d\n",row-row_true-1);

        fclose(fpt_hex);
        fclose(fpt_aqi);
        return 0;

}

/*=============================================DAT TO CSV=====================================================*/

/*check packet in dat file*/
int checkPacket(unsigned char byteArray[16]){
    unsigned char byteBuffer[8];
    int buf;
    sprintf(byteBuffer,"%02X",byteArray[14]);
    sscanf(byteBuffer,"%x",&buf);
    int end = *((int*)&buf);
    sprintf(byteBuffer,"%02X",byteArray[0]);
    sscanf(byteBuffer,"%x",&buf);
    int start = *((int*)&buf);
    sprintf(byteBuffer,"%02X",byteArray[1]);
    sscanf(byteBuffer,"%x",&buf);
    int length = *((int*)&buf);
    if((checkSum(byteArray,14) - byteArray[13] == 0) && (end - 255 == 0) && (start - 0 == 0) && (length - 15 == 0)){
        return 0;
    }
    return 1;
}


/*pollution*/
void pollution(float average,char str[]){
    if((average >=0) && (average < 12)){
        strcpy(str,"Good");
    }
    else if((average >= 12) && (average < 35.5)){
       strcpy(str,"Moderate");
    }
    else if((average >= 35.5) && (average < 55.5)){
        strcpy(str,"Slightly unhealthy");
    }
    else if((average >= 55.5) && (average <150.5)){
        strcpy(str,"Unhealthy");
    }
    else if((average >= 150.5) && (average < 250.5)){
        strcpy(str,"Very unhealthy");
    }
    else if((average >= 250.5) && (average < 350.5)){
        strcpy(str,"Hazardous");
    }
    else if((average >= 350.5) && (average < 550.5)){
        strcpy(str,"Extremely hazardous");
    }
}

int maxID;
int row_number;

void countRow(FILE* fpt_hex){
    char buffer[1024];
    int collumn;
    unsigned char byteArray[16];
    unsigned char byteBuffer[8];
    int buf;
    int id = 0;
    row_number = 0;
    maxID = 0;
    while(fgets(buffer,1024,fpt_hex)){
        collumn = 0;
        row_number ++;
        char* token;
        token = strtok(buffer," ");
        while(token != NULL){
            sscanf(token,"%02X",&byteArray[collumn]);
            token = strtok(NULL," ");
            collumn ++;
        }
        sprintf(byteBuffer,"%02X",byteArray[2]);
        sscanf(byteBuffer,"%x",&buf);
        id = *((int*)&buf);
        if(id > maxID){
            maxID =id;
        }
    }
    maxID ++;
    row_number ++;
    fclose(fpt_hex);
    return;
}

struct info{
    int id;
    time_t time;
    float values;
    int Aqi;
    };


/*Sorting*/

/*Ascending*/
void InterchangeSortingAsc(struct info lines[],int row_true,long int buf1[],long int buf2[],long int buf3[],int checkParam){
    struct info temp;
    long int tmp1,tmp2,tmp3;
    if(checkParam > 0){   
        for(int i=1;i<=row_true-1;i++){
            for(int j=i+1;j<=row_true;j++){
                if(buf1[i] > buf1[j]){
                    temp = lines[i];
                    lines[i] = lines[j];
                    lines[j] = temp;
                    tmp1 = buf1[i];
                    buf1[i] = buf1[j];
                    buf1[j] = tmp1;
                    tmp2 = buf2[i];
                    buf2[i] = buf2[j];
                    buf2[j] = tmp2;
                    tmp3 = buf3[i];
                    buf3[i] = buf3[j];
                    buf3[j] = tmp3;
                }
            }
        }
        if(checkParam > 1){
        for(int i=1;i<=row_true-1;i++){
            for(int j=i+1;j<=row_true;j++){
                if((buf1[i] == buf1[j]) && (buf2[i] > buf2[j])){
                    temp = lines[i];
                    lines[i] = lines[j];
                    lines[j] = temp;
                    tmp1 = buf1[i];
                    buf1[i] = buf1[j];
                    buf1[j] = tmp1;
                    tmp2 = buf2[i];
                    buf2[i] = buf2[j];
                    buf2[j] = tmp2;
                    tmp3 = buf3[i];
                    buf3[i] = buf3[j];
                    buf3[j] = tmp3;
                }
            }
        }
            if(checkParam > 2){
                for(int i=1;i<=row_true-1;i++){
                    for(int j=i+1;j<=row_true;j++){
                        if((buf1[i] == buf1[j]) && (buf2[i] == buf2[j]) && (buf3[i] > buf3[j])){
                            temp = lines[i];
                            lines[i] = lines[j];
                            lines[j] = temp;
                            tmp1 = buf1[i];
                            buf1[i] = buf1[j];
                            buf1[j] = tmp1;
                            tmp2 = buf2[i];
                            buf2[i] = buf2[j];
                            buf2[j] = tmp2;
                            tmp3 = buf3[i];
                            buf3[i] = buf3[j];
                            buf3[j] = tmp3;
                        }
                    }
                }
            }
        }
    }
}

/*ascending*/
void BubbleSortingAsc(struct info lines[],int row_true,long int buf1[],long int buf2[],long int buf3[],int checkParam){
    struct info temp;
    long int tmp1,tmp2,tmp3;
    if(checkParam > 0){   
        for(int i=1;i<=row_true-1;i++){
            for(int j=row_true;j>i;j--){
                if(buf1[i] > buf1[j]){
                    temp = lines[i];
                    lines[i] = lines[j];
                    lines[j] = temp;
                    tmp1 = buf1[i];
                    buf1[i] = buf1[j];
                    buf1[j] = tmp1;
                    tmp2 = buf2[i];
                    buf2[i] = buf2[j];
                    buf2[j] = tmp2;
                    tmp3 = buf3[i];
                    buf3[i] = buf3[j];
                    buf3[j] = tmp3;
                }
            }
        }
        if(checkParam > 1){
        for(int i=1;i<=row_true-1;i++){
            for(int j=row_true;j>i;j--){
                if((buf1[i] == buf1[j]) && (buf2[i] > buf2[j])){
                    temp = lines[i];
                    lines[i] = lines[j];
                    lines[j] = temp;
                    tmp1 = buf1[i];
                    buf1[i] = buf1[j];
                    buf1[j] = tmp1;
                    tmp2 = buf2[i];
                    buf2[i] = buf2[j];
                    buf2[j] = tmp2;
                    tmp3 = buf3[i];
                    buf3[i] = buf3[j];
                    buf3[j] = tmp3;
                }
            }
        }
            if(checkParam > 2){
                for(int i=1;i<=row_true-1;i++){
                    for(int j=row_true;j>i;j--){
                        if((buf1[i] == buf1[j]) && (buf2[i] == buf2[j]) && (buf3[i] > buf3[j])){
                            temp = lines[i];
                            lines[i] = lines[j];
                            lines[j] = temp;
                            tmp1 = buf1[i];
                            buf1[i] = buf1[j];
                            buf1[j] = tmp1;
                            tmp2 = buf2[i];
                            buf2[i] = buf2[j];
                            buf2[j] = tmp2;
                            tmp3 = buf3[i];
                            buf3[i] = buf3[j];
                            buf3[j] = tmp3;
                        }
                    }
                }
            }
        }
    }
}

/*Descending*/
void InterchangeSortingDsc(struct info lines[],int row_true,long int buf1[],long int buf2[],long int buf3[],int checkParam){
    struct info temp;
    long int tmp1,tmp2,tmp3;
    if(checkParam > 0){   
        for(int i=1;i<=row_true-1;i++){
            for(int j=i+1;j<=row_true;j++){
                if(buf1[i] < buf1[j]){
                    temp = lines[i];
                    lines[i] = lines[j];
                    lines[j] = temp;
                    tmp1 = buf1[i];
                    buf1[i] = buf1[j];
                    buf1[j] = tmp1;
                    tmp2 = buf2[i];
                    buf2[i] = buf2[j];
                    buf2[j] = tmp2;
                    tmp3 = buf3[i];
                    buf3[i] = buf3[j];
                    buf3[j] = tmp3;
                }
            }
        }
        if(checkParam > 1){
        for(int i=1;i<=row_true-1;i++){
            for(int j=i+1;j<=row_true;j++){
                if((buf1[i] == buf1[j]) && (buf2[i] < buf2[j])){
                    temp = lines[i];
                    lines[i] = lines[j];
                    lines[j] = temp;
                    tmp1 = buf1[i];
                    buf1[i] = buf1[j];
                    buf1[j] = tmp1;
                    tmp2 = buf2[i];
                    buf2[i] = buf2[j];
                    buf2[j] = tmp2;
                    tmp3 = buf3[i];
                    buf3[i] = buf3[j];
                    buf3[j] = tmp3;
                }
            }
        }
            if(checkParam > 2){
                for(int i=1;i<=row_true-1;i++){
                    for(int j=i+1;j<=row_true;j++){
                        if((buf1[i] == buf1[j]) && (buf2[i] == buf2[j]) && (buf3[i] < buf3[j])){
                            temp = lines[i];
                            lines[i] = lines[j];
                            lines[j] = temp;
                            tmp1 = buf1[i];
                            buf1[i] = buf1[j];
                            buf1[j] = tmp1;
                            tmp2 = buf2[i];
                            buf2[i] = buf2[j];
                            buf2[j] = tmp2;
                            tmp3 = buf3[i];
                            buf3[i] = buf3[j];
                            buf3[j] = tmp3;
                        }
                    }
                }
            }
        }
    }
}

/*descending*/
void BubbleSortingDsc(struct info lines[],int row_true,long int buf1[],long int buf2[],long int buf3[],int checkParam){
    struct info temp;
    long int tmp1,tmp2,tmp3;
    if(checkParam > 0){   
        for(int i=1;i<=row_true-1;i++){
            for(int j=row_true;j>i;j--){
                if(buf1[i] < buf1[j]){
                    temp = lines[i];
                    lines[i] = lines[j];
                    lines[j] = temp;
                    tmp1 = buf1[i];
                    buf1[i] = buf1[j];
                    buf1[j] = tmp1;
                    tmp2 = buf2[i];
                    buf2[i] = buf2[j];
                    buf2[j] = tmp2;
                    tmp3 = buf3[i];
                    buf3[i] = buf3[j];
                    buf3[j] = tmp3;
                }
            }
        }
        if(checkParam > 1){
            for(int i=1;i<=row_true-1;i++){
                for(int j=row_true;j>i;j--){
                if((buf1[i] == buf1[j]) && (buf2[i] < buf2[j])){
                    temp = lines[i];
                    lines[i] = lines[j];
                    lines[j] = temp;
                    tmp1 = buf1[i];
                    buf1[i] = buf1[j];
                    buf1[j] = tmp1;
                    tmp2 = buf2[i];
                    buf2[i] = buf2[j];
                    buf2[j] = tmp2;
                    tmp3 = buf3[i];
                    buf3[i] = buf3[j];
                    buf3[j] = tmp3;
                }
            }
        }
            if(checkParam > 2){
                for(int i=1;i<=row_true-1;i++){
                    for(int j=row_true;j>i;j--){
                        if((buf1[i] == buf1[j]) && (buf2[i] == buf2[j]) && (buf3[i] < buf3[j])){
                            temp = lines[i];
                            lines[i] = lines[j];
                            lines[j] = temp;
                            tmp1 = buf1[i];
                            buf1[i] = buf1[j];
                            buf1[j] = tmp1;
                            tmp2 = buf2[i];
                            buf2[i] = buf2[j];
                            buf2[j] = tmp2;
                            tmp3 = buf3[i];
                            buf3[i] = buf3[j];
                            buf3[j] = tmp3;
                        }
                    }
                }
            }
        }
    }
}

/*convert from dat file to csv file*/
int dat2csv(int argc, char *argv[],FILE* fpt_log_error,FILE* fpt_log_run,int checkArgument,int chParam,int chPriority){
    
    /*file dat*/
    FILE* fpt_hex = fopen(argv[1],"r");

        if(fpt_hex == NULL){
            printf("Error opening dat file\n");
            fprintf(fpt_log_error,"Error 02: denied access %s\n",argv[1]);
            
            return 1;
        }

    /*file dust_aqi*/
    FILE* fpt_aqi = fopen(argv[2],"w+");
        
        if(fpt_aqi == NULL){
            printf("Error override csv file\n");
            fprintf(fpt_log_error,"Error 02: denied access %s\n",argv[2]);
            fclose(fpt_hex);
            return 1;
        }


    /*csv*/
    fprintf(fpt_aqi,"id,time,values,aqi,pollution\n");

    countRow(fpt_hex);
    FILE* fpt_reopen = fopen(argv[1],"r");

    unsigned char byteArray[16];
    unsigned char byteBuffer[8];
    
    long int buf1,buf2,buf3,buf4;
    int aqi;
    int id;
    float average;
    time_t t;
    struct tm *t_info;
    char t_buffer[80];

    /*check time*/
    time_t t_now;
    t_now = time(&t_now);

    char buffer[1024];
    int row = 0;
    int row_true = 0;
    int collumn = 0;

    char pollut[25];

    /*for check duplicate*/
    long int duplicate[maxID][row_number];
    int checkDup = 0;
    for(int i=0;i<maxID;i++){
        for(int j=0;j<row_number;j++){
            duplicate[i][j] = 0;
        }
    }

    /*for sorting*/
    /*interchange*/
    long int idBuf[row_number];
    long int timeBuf[row_number];
    long int valueBuf[row_number];
    /*bubblesort*/
    long int idBuf2[row_number];
    long int timeBuf2[row_number];
    long int valueBuf2[row_number];

    /*print after sorting*/
    struct info tmp, lines[row_number],lines2[row_number];

    

    /*"00 0F 01 63 81 57 3C 42 48 66 66 00 89 9A FF";
       0  1  2  3  4  5  6  7  8  9  10 11 12 13 14
           pk id |   time  | | average | |aqi| cs end 
    */

    while(fgets(buffer,1024,fpt_reopen)){
        collumn = 0;
        checkDup = 0; 
        row ++;
        char* token;
        token = strtok(buffer," ");
        while(token != NULL){
            sscanf(token,"%02X",&byteArray[collumn]);
            token = strtok(NULL," ");
            collumn ++;
        }
        if(collumn == 15){
            if(checkPacket(byteArray) == 0){
                /*byte time*/
                sprintf(byteBuffer,"%02X%02X%02X%02X",byteArray[3],byteArray[4],byteArray[5],byteArray[6]);
                sscanf(byteBuffer, "%x", &buf1);
                t = *((int*)&buf1);
                
                /*byte average*/
                sprintf(byteBuffer,"%02X%02X%02X%02X",byteArray[7],byteArray[8],byteArray[9],byteArray[10]);
                sscanf(byteBuffer,"%x",&buf2);
                average = *((float*)&buf2);
                
                /*byte aqi*/
                sprintf(byteBuffer,"%02X%02X",byteArray[11],byteArray[12]);
                sscanf(byteBuffer,"%x",&buf3);
                aqi = *((int*)&buf3);

                /*byte ID*/
                sprintf(byteBuffer,"%02X",byteArray[2]);
                sscanf(byteBuffer,"%x",&buf4);
                id = *((int*)&buf4);

                for(int i=1;i<row;i++){
                    if(t == duplicate[id][i]){
                        checkDup = 1;
                        fprintf(fpt_log_error,"Error 05: duplicated data at lines %d, %d\n",i,row);
                        break;
                    }
                    else{
                        duplicate[id][row] = t;
                    }
                }

                /*check aqi && (t_now > t)*/
                if((abs(aqi - AQI(average))<2) && (t_now > t)){
                    if(checkDup == 0){
                    row_true++;
                    
                    /*info interchange*/
                    lines[row_true].id = id;
                    lines[row_true].values = average;
                    lines[row_true].Aqi = aqi;
                    lines[row_true].time = t;

                    idBuf[row_true] = buf4;
                    timeBuf[row_true] = buf1;
                    valueBuf[row_true] = buf2;

                    /*info bubble*/
                    lines2[row_true].id = id;
                    lines2[row_true].values = average;
                    lines2[row_true].Aqi = aqi;
                    lines2[row_true].time = t;

                    idBuf2[row_true] = buf4;
                    timeBuf2[row_true] = buf1;
                    valueBuf2[row_true] = buf2;
                    }
                }
                else {
                    fprintf(fpt_log_error,"Error 06: invalid data packet at line %d\n",row);
                }
            }
            else{
                fprintf(fpt_log_error,"Error 06: invalid data packet at line %d\n",row);
            }
        }
        else if(collumn != 15){
            
            fprintf(fpt_log_error,"Error 03: invalid file format at line %d\n",row);
        }

    /*end while*/
    }

    /*CLOCK*/
    clock_t start,start2,end,end2;
    double timeUse,timeUse2;

    /*SORTING*/
    if(checkArgument == 4){
        start = clock();
        InterchangeSortingAsc(lines,row_true,idBuf,timeBuf,valueBuf,3);
        end = clock();
        timeUse = (double)(end - start)/CLOCKS_PER_SEC;
        start2 = clock();
        BubbleSortingAsc(lines2,row_true,idBuf2,timeBuf2,valueBuf2,3);
        end2 = clock();
        timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
    }
    if(checkArgument == 5){
        start = clock();
        InterchangeSortingDsc(lines,row_true,idBuf,timeBuf,valueBuf,3);
        end = clock();
        timeUse = (double)(end - start)/CLOCKS_PER_SEC;
        start2 = clock();
        BubbleSortingDsc(lines2,row_true,idBuf2,timeBuf2,valueBuf2,3);
        end2 = clock();
        timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
    }
    if(checkArgument == 6){
        
        if(chPriority > 0 && chPriority < 3 ){
            start = clock();
            InterchangeSortingAsc(lines,row_true,idBuf,timeBuf,valueBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingAsc(lines2,row_true,idBuf2,timeBuf2,valueBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
            
        }
        if(chPriority == 3){
            start = clock();
            InterchangeSortingAsc(lines,row_true,idBuf,valueBuf,timeBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingAsc(lines2,row_true,idBuf2,valueBuf2,timeBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
            
        }
        if(chPriority > 3 && chPriority < 6){
            start = clock();
            InterchangeSortingAsc(lines,row_true,timeBuf,idBuf,valueBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingAsc(lines2,row_true,timeBuf2,idBuf2,valueBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
            
        }
        if(chPriority == 6){
            start = clock();
            InterchangeSortingAsc(lines,row_true,timeBuf,valueBuf,idBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingAsc(lines2,row_true,timeBuf2,valueBuf2,idBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
            
        }
        if(chPriority > 6 && chPriority < 8){
            start = clock();
            InterchangeSortingAsc(lines,row_true,valueBuf,idBuf,timeBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingAsc(lines2,row_true,valueBuf2,idBuf2,timeBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
            
        }
        if(chPriority == 8){
            start = clock();
            InterchangeSortingAsc(lines,row_true,valueBuf,timeBuf,idBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingAsc(lines2,row_true,valueBuf2,timeBuf2,idBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
            
        }
    }
    if(checkArgument == 7){
        
        if(chPriority > 0 && chPriority < 3 ){
            start = clock();
            InterchangeSortingDsc(lines,row_true,idBuf,timeBuf,valueBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingDsc(lines2,row_true,idBuf2,timeBuf2,valueBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
        }
        if(chPriority == 3){
            start = clock();
            InterchangeSortingDsc(lines,row_true,idBuf,valueBuf,timeBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingDsc(lines2,row_true,idBuf2,valueBuf2,timeBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
        }
        if(chPriority > 3 && chPriority < 6){
            start = clock();
            InterchangeSortingDsc(lines,row_true,timeBuf,idBuf,valueBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingDsc(lines2,row_true,timeBuf2,idBuf2,valueBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
        }
        if(chPriority == 6){
            start = clock();
            InterchangeSortingDsc(lines,row_true,timeBuf,valueBuf,idBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingDsc(lines2,row_true,timeBuf2,valueBuf2,idBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
        }
        if(chPriority > 6 && chPriority < 8){
            start = clock();
            InterchangeSortingDsc(lines,row_true,valueBuf,idBuf,timeBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingDsc(lines2,row_true,valueBuf2,idBuf2,timeBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
        }
        if(chPriority == 8){
            start = clock();
            InterchangeSortingDsc(lines,row_true,valueBuf,timeBuf,idBuf,chParam);
            end = clock();
            timeUse = (double)(end - start)/CLOCKS_PER_SEC;
            start2 = clock();
            BubbleSortingDsc(lines2,row_true,valueBuf2,timeBuf2,idBuf2,chParam);
            end2 = clock();
            timeUse2 = (double)(end2 - start2)/CLOCKS_PER_SEC;
        }
    }

    for(int i=1;i <= row_true;i++){
        pollution(lines[i].values,pollut);
        t_info = localtime(&lines[i].time);
        strftime(t_buffer,80,"%Y:%m:%d %H:%M:%S",t_info);
        fprintf(fpt_aqi,"%d,%s,%.1f,%d,%s\n",lines[i].id,t_buffer,lines[i].values,lines[i].Aqi,pollut);
    }


    /*log*/

    /*log*/
    fprintf(fpt_log_run,"Total number of rows: %d\n",row);
    fprintf(fpt_log_run,"Succesfully converted rows: %d\n",row_true);
    fprintf(fpt_log_run,"Error rows: %d\n",row - row_true);

    /*if sort*/
    if(checkArgument > 3){
        fprintf(fpt_log_run,"Sorting algorithm interchange[ms]: %.f\n",timeUse*1000);
        fprintf(fpt_log_run,"Sorting algorithm bubble[ms]: %.f\n",timeUse2*1000);
    }

    fclose(fpt_reopen);
    fclose(fpt_aqi);
    return 0;

/*end function*/
}

/*=====================================================================MAIN===================================================================*/


int main(int argc, char *argv[]){

        /*   Error 01: Invalid command
        *    Error 02: denied access FILENAME
        *    Error 03: invalid file format at line
        *    Error 04: invalid data or data is missing at line
        *    Error 05: duplicated data at lines
        *    Error 06: invalid data packet at line
        */

    /*log file*/
    FILE* fpt_log_error = fopen("dust_convert_error.log","w+");
    /*error opening log file*/
        if(fpt_log_error == NULL){
            printf("Error opening log file\n");
            return 1;
        }
    FILE* fpt_log_run = fopen("dust_convert_run.log","w+");
    /*error opening log file*/
        if(fpt_log_run == NULL){
            printf("Error opening log file\n");
            return 1;
        }
    if(argc != 3 && argc != 5 && argc != 6){
        printf("Invalid command\n");
        fprintf(fpt_log_error,"Error 01: Invalid command\n");
        fclose(fpt_log_error);
        fclose(fpt_log_run);
        return 1;
    }

    int checkParam = checkParams(argv[4]); 
   
    int check = checkArgument(argc,argv,checkParam);
    
    if(check < 1){
        printf("Invalid command\n");
        fprintf(fpt_log_error,"Error 01: Invalid command\n");
        fclose(fpt_log_error);
        fclose(fpt_log_run);
        return 1;
    }

    int convert = 0;

    if(check == 1){
        convert = csv2dat(argc,argv,fpt_log_error,fpt_log_run);
    }
    if(check > 1){
        convert = dat2csv(argc,argv,fpt_log_error,fpt_log_run,check,checkParam,prio);
    }

    if(convert != 0){
        printf("Error converting\n");
    }    
    fclose(fpt_log_error);
    fclose(fpt_log_run); 
    return 0;
}