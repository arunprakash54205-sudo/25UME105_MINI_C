// Bank-account program reads a random-access file sequentially,
// updates data already written to the file, creates new data to
// be placed in the file, and deletes data previously in the file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clientData structure definition
struct clientData
{
    unsigned int acctNum; // account number
    char lastName[15];    // account last name
    char firstName[10];   // account first name
    double balance;       // account balance
};                        // end structure clientData

// prototypes
unsigned int enterChoice(void);
void textFile(FILE *readPtr);
void updateRecord(FILE *fPtr);
void newRecord(FILE *fPtr);
void deleteRecord(FILE *fPtr);
void listRecords(FILE *readPtr);
void searchRecordByName(FILE *readPtr);
void showTotalBalance(FILE *readPtr);

// Helper function prototypes for functional decomposition
unsigned int getValidAccountNum(const char *prompt);
void readRecord(FILE *fPtr, unsigned int accountNum, struct clientData *client);
void writeRecord(FILE *fPtr, unsigned int accountNum, const struct clientData *client);

int main(int argc, char *argv[])
{
    FILE *cfPtr;         // clients.dat file pointer
    unsigned int choice; // user's choice

    // fopen opens the file; exits if file cannot be opened
    if ((cfPtr = fopen("clients.dat", "rb+")) == NULL)
    {
        // Try creating it since it might not exist
        if ((cfPtr = fopen("clients.dat", "wb+")) == NULL)
        {
            printf("%s: File could not be opened.\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        
        // Initialize file with 100 blank records
        struct clientData blankClient = {0, "", "", 0.0};
        for (unsigned int i = 1; i <= 100; ++i)
        {
            fwrite(&blankClient, sizeof(struct clientData), 1, cfPtr);
        }
        rewind(cfPtr); // Reset pointer to start of file after initialization
    }

    // enable user to specify action
    while ((choice = enterChoice()) != 8)
    {
        switch (choice)
        {
        case 1:
            textFile(cfPtr);
            break;
        case 2:
            updateRecord(cfPtr);
            break;
        case 3:
            newRecord(cfPtr);
            break;
        case 4:
            deleteRecord(cfPtr);
            break;
        case 5:
            listRecords(cfPtr);
            break;
        case 6:
            searchRecordByName(cfPtr);
            break;
        case 7:
            showTotalBalance(cfPtr);
            break;
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    fclose(cfPtr); // fclose closes the file
    return 0;
} // end main

// create formatted text file for printing
void textFile(FILE *readPtr)
{
    FILE *writePtr; // accounts.txt file pointer
    struct clientData clients[100]; // Refactored for speed: block read instead of individual reads

    // fopen opens the file; exits if file cannot be opened
    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } 
    else
    {
        rewind(readPtr); // sets pointer to beginning of file
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        // bulk I/O for better performance (Speed Optimization)
        size_t recordsRead = fread(clients, sizeof(struct clientData), 100, readPtr);
        
        for (size_t i = 0; i < recordsRead; i++)
        {
            if (clients[i].acctNum != 0)
            {
                fprintf(writePtr, "%-6u%-16s%-11s%10.2f\n", clients[i].acctNum, clients[i].lastName, clients[i].firstName, clients[i].balance);
            }
        }

        fclose(writePtr); // fclose closes the file
        puts("Successfully exported accounts to accounts.txt.");
    }                     
} // end function textFile

// update balance in record
void updateRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int account = getValidAccountNum("Enter account to update ( 1 - 100 ): ");
    if (account == 0) return; // Invalid or aborted

    readRecord(fPtr, account, &client);

    // display error if account does not exist
    if (client.acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
    }
    else
    { // update record
        printf("%-6u%-16s%-11s%10.2f\n\n", client.acctNum, client.lastName, client.firstName, client.balance);

        double transaction;
        // request transaction amount from user
        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        if (scanf("%lf", &transaction) != 1) {
            int c; while ((c = getchar()) != '\n' && c != EOF);
            puts("Invalid transaction amount.");
            return;
        }
        
        client.balance += transaction; // update record balance

        printf("%-6u%-16s%-11s%10.2f\n", client.acctNum, client.lastName, client.firstName, client.balance);

        writeRecord(fPtr, account, &client);
        puts("Account updated successfully.");
    } // end else
} // end function updateRecord

// delete an existing record
void deleteRecord(FILE *fPtr)
{
    struct clientData client;                       
    struct clientData blankClient = {0, "", "", 0.0}; 
    
    unsigned int accountNum = getValidAccountNum("Enter account number to delete ( 1 - 100 ): ");
    if (accountNum == 0) return;

    readRecord(fPtr, accountNum, &client);

    // display error if record does not exist
    if (client.acctNum == 0)
    {
        printf("Account %u does not exist.\n", accountNum);
    } 
    else
    { // delete record
        writeRecord(fPtr, accountNum, &blankClient);
        printf("Account %u deleted successfully.\n", accountNum);
    } 
} // end function deleteRecord

// create and insert record
void newRecord(FILE *fPtr)
{
    struct clientData client = {0, "", "", 0.0};
    unsigned int accountNum = getValidAccountNum("Enter new account number ( 1 - 100 ): ");
    if (accountNum == 0) return;

    readRecord(fPtr, accountNum, &client);

    // display error if account already exists
    if (client.acctNum != 0)
    {
        printf("Account #%u already contains information.\n", client.acctNum);
    } 
    else
    { // create record
        printf("%s", "Enter lastname, firstname, balance\n? ");
        if (scanf("%14s%9s%lf", client.lastName, client.firstName, &client.balance) != 3) {
            int c; while ((c = getchar()) != '\n' && c != EOF);
            puts("Invalid input format.");
            return;
        }

        client.acctNum = accountNum;
        writeRecord(fPtr, accountNum, &client);
        printf("Account %u added successfully.\n", accountNum);
    } 
} // end function newRecord

// list all accounts
void listRecords(FILE *readPtr)
{
    struct clientData clients[100]; // Refactored for speed: block read

    rewind(readPtr); // sets pointer to beginning of file
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");

    // bulk I/O for speed optimization
    size_t recordsRead = fread(clients, sizeof(struct clientData), 100, readPtr);
    for (size_t i = 0; i < recordsRead; i++)
    {
        if (clients[i].acctNum != 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n", clients[i].acctNum, clients[i].lastName, clients[i].firstName, clients[i].balance);
        }
    }
    printf("---------------------------------------------\n\n");
} // end function listRecords

// search account by last name
void searchRecordByName(FILE *readPtr)
{
    char searchName[15];
    struct clientData clients[100];
    int found = 0;

    printf("Enter last name to search: ");
    if (scanf("%14s", searchName) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        puts("Invalid input.");
        return;
    }

    rewind(readPtr);
    size_t recordsRead = fread(clients, sizeof(struct clientData), 100, readPtr);
    
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");
    for (size_t i = 0; i < recordsRead; i++)
    {
        if (clients[i].acctNum != 0 && strcmp(clients[i].lastName, searchName) == 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n", clients[i].acctNum, clients[i].lastName, clients[i].firstName, clients[i].balance);
            found = 1;
        }
    }
    if (!found) {
        printf("No accounts found with last name '%s'.\n", searchName);
    }
    printf("---------------------------------------------\n\n");
}

// show total balance of all accounts
void showTotalBalance(FILE *readPtr)
{
    struct clientData clients[100];
    double total = 0.0;

    rewind(readPtr);
    size_t recordsRead = fread(clients, sizeof(struct clientData), 100, readPtr);
    
    for (size_t i = 0; i < recordsRead; i++)
    {
        if (clients[i].acctNum != 0)
        {
            total += clients[i].balance;
        }
    }
    printf("\n---------------------------------------------\n");
    printf("Total Bank Balance: %.2f\n", total);
    printf("---------------------------------------------\n\n");
}

// enable user to input menu choice
unsigned int enterChoice(void)
{
    unsigned int menuChoice; // variable to store user's choice
    // display available options
    printf("%s", "\nEnter your choice\n"
                 "1 - store a formatted text file of accounts called\n"
                 "    \"accounts.txt\" for printing\n"
                 "2 - update an account\n"
                 "3 - add a new account\n"
                 "4 - delete an account\n"
                 "5 - list all accounts\n"
                 "6 - search account by last name\n"
                 "7 - show total bank balance\n"
                 "8 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1) {
        // clear input buffer if invalid entry is made
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        menuChoice = 0; // set to invalid choice
    }
    return menuChoice;
} // end function enterChoice

// --- Helper Functions for Functional Decomposition ---

// prompts user and returns a valid account number (1-100), or 0 if invalid
unsigned int getValidAccountNum(const char *prompt)
{
    unsigned int accountNum;
    printf("%s", prompt);
    if (scanf("%u", &accountNum) != 1)
    {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        return 0; 
    }
    if (accountNum < 1 || accountNum > 100)
    {
        puts("Invalid account number.");
        return 0;
    }
    return accountNum;
}

// seeks to the correct position and reads a record
void readRecord(FILE *fPtr, unsigned int accountNum, struct clientData *client)
{
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fread(client, sizeof(struct clientData), 1, fPtr);
}

// seeks to the correct position and writes a record
void writeRecord(FILE *fPtr, unsigned int accountNum, const struct clientData *client)
{
    fseek(fPtr, (accountNum - 1) * sizeof(struct clientData), SEEK_SET);
    fwrite(client, sizeof(struct clientData), 1, fPtr);
}