#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct file_read{  // my basic readed files storage struct
	void 	*Data; // all data in the file
	uint64_t Len;  // lenght of the file for further allocate operations
};

struct Record{     // my main struct for storaging data.
    char name[64]; //utf16
    char surname[32]; //utf8 - since we read all the data in UTF-8 format, there is no need to change anything in surname.
    char gender;
    char email[32];
    char phone_number[16];
    char address[32];
    char level_of_education[8];
    unsigned int income_level; // given little-endian
    unsigned int expenditure; // given big-endian
    char currency_unit[16];
    char currentMood[32];
    float height;
    unsigned int weight;
};

uint32_t swapEndians(uint32_t val) // swapping operations from big to little or little to big endians.
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF); // basicly changing the positions of bytes
    return (val << 16) | (val >> 16);
}

struct file_read read_entire_file(const char* FileName); // defining my file read method here.

// assumes argument output can hold output xml result!
void
convert_record_to_xml(struct Record *record, char *output, uint32_t *out_len){ // my most important method for helping placing the data to their correct positions.

    if (record->name[0] == 0) // basicly exit the function if name has no char inside
    {
        *out_len = 0;
        return;
    }    
	
	uint32_t i = 0;
	i += sprintf(output + i, "\t\t<name>%s</name>\n", 				record->name); // i is for calculating the total number of characters for one employee in that element of 
	i += sprintf(output + i, "\t\t<surname>%s</surname>\n", 			record->surname);  // struct array (record)
	i += sprintf(output + i, "\t\t<gender>%c</gender>\n", 			record->gender);
	i += sprintf(output + i, "\t\t<email>%s</email>\n", 				record->email);
	i += sprintf(output + i, "\t\t<phone_number>%s</phone_number>\n", record->phone_number);
	i += sprintf(output + i, "\t\t<address>%s</address>\n", 			record->address);
	i += sprintf(output + i, "\t\t<level_of_education>%s</level_of_education>\n", 	record->level_of_education);
	i += sprintf(output + i, "\t\t<income_level bigEnd=\"%u\">%u</income_level>\n", 	swapEndians(record->income_level), record->income_level);
	i += sprintf(output + i, "\t\t<expenditure bigEnd=\"%u\">%u</expenditure>\n", 	record->expenditure, swapEndians(record->expenditure));
	i += sprintf(output + i, "\t\t<currency_unit>%s</currency_unit>\n", 				record->currency_unit);
	i += sprintf(output + i, "\t\t<currentMood>%s</currentMood>\n", 					record->currentMood);
	i += sprintf(output + i, "\t\t<height>%.2f</height>\n", 				record->height);
	i += sprintf(output + i, "\t\t<weight>%u</weight>\n", 					record->weight);

	*out_len = i; // modifying the total lenght of readed chars.
}

int main(int argc, char *argv[]){

    const char *input_fn = argv[1];  // my first argument for asking the user to be readed file in terminal. comment this and uncomment below line for running classically (F5)
    //const char *input_fn = "records.dat";

    const char *output_fn = argv[2]; // my second argument for asking the user to be created file in terminal. comment this and uncomment below line for running classically (F5)
    //const char *output_fn = "records.xml";

    char temp_inp[50];

	struct file_read fr = read_entire_file(input_fn); // creating my struct for holding the all data
	
	if(fr.Data == NULL) // close the program with going bail if no Data readed.
		goto bail;

	struct Record *records = (struct Record*)fr.Data; // my main struct variable for holding the data.
	uint32_t rc 	= fr.Len/sizeof(struct Record);   // counting the total number of employees.
	char bf[2056];                                    // basic buffer for storaging some further characters.
    char* bigBuffer = (char*)malloc(fr.Len * 5);      // my big buffer for storaging all the xml output.
    char temp[64];                                    // for some temporary operations.

	uint32_t len;

    for (size_t i = 0; i < strlen(input_fn); i++) // basic loop for removing the characters after '.' in file name.
    {
        if (input_fn[i] == '.') break;
        temp_inp[i] = input_fn[i];
    }    

    printf("<%s>\n", temp_inp); // printing and

    sprintf(bigBuffer, "<%s>\n", temp_inp); // writing the file name.

    for (size_t i = 1; i < rc - 4; i++) // printing the characters into output and writing into xml file.
    {
        printf("\t<row id=\"%d\">\n", i); // printing and

        sprintf(temp, "\t<row id=\"%d\">\n", i); // writing the row number.
        strncat(bigBuffer, temp, strlen(temp));

        convert_record_to_xml(&records[i], bf, &len); // sending my variables into my method for converting them into xml and getting some information.
        printf("%s\t</row>\n", bf); // printing and

        strncat(bigBuffer, bf, len); // writing closing row
        strncat(bigBuffer, "\t</row>\n", 10);
    }

    printf("</%s>", temp_inp);

    sprintf(temp, "</%s>", temp_inp);
    strncat(bigBuffer, temp, strlen(input_fn) + 10);

    FILE *fp;
    fp = fopen(output_fn, "w");

    fwrite(bigBuffer, 1, strlen(bigBuffer), fp); // printing all the correct data into xml at once.

    fclose(fp);

	if(records == NULL)
		goto bail;
	
	bail:
	// error handling and stuff
	return 0;
}

struct file_read
read_entire_file(const char* FileName) { // my old function for reading data from any file(.bin, .txt, .dat, etc. etc.). literally you can use it anywhere in c :)
    struct file_read Result = { 0 };     // initilazing the struct NULL
    FILE *File = fopen(FileName, "rb");  // reading the file
    if (File != 0) {                     // work if reading was succesful
        
        if(0 == fseek(File, 0, SEEK_END)){ // checking if the file position setted correctly for giving offset with fseek()
            Result.Len = ftell(File);      // returning the current file position of the given stream.
            if(0 != fseek(File, 0, SEEK_SET)){ // basic error
                printf("Unable to set file pointer to beginning of the file\n");
                Result.Len = 0;
            }
        }            
        
        if (Result.Len == 0){ // exiting if up condition is correct.
            printf("Length was zero, early terminating readfile for file %s\n", FileName);
            return Result;
        }
        
        Result.Data = malloc((size_t)Result.Len); // allocating the memory for all the data in the file.
        if (Result.Data) {
            int fresult = fread(Result.Data, Result.Len, 1, File); // fresult is for boolean value 1 or 0.
            if (1 == fresult) {
                // NOTE success
            }
            else {
                printf("Unable to read file %s, err code %d\n", FileName, ferror(File)); // error if 0
                free(Result.Data);
                Result.Data = 0;
                Result.Len = 0;
            }
        }
        fclose(File);

    }
    else {
        printf("Can't create file: %s\n", FileName);
    }
    return Result;
}