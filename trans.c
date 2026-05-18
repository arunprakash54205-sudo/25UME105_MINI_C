// Bank-account program refactored for maximum efficiency.
// It loads the random-access file into memory sequentially at startup,
// performs all reads/updates/inserts/deletes purely in high-speed RAM,
// and saves the updated data back to the file upon exiting.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RECORDS 100

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
void textFile(const struct clientData clients[]);
void updateRecord(struct clientData clients[]);
void newRecord(struct clientData clients[]);
void deleteRecord(struct clientData clients[]);
void listRecords(const struct clientData clients[]);
void searchRecordByName(const struct clientData clients[]);
void showTotalBalance(const struct clientData clients[]);
void saveAndExit(const struct clientData clients[], const char *filename);

// Helper function prototypes for functional decomposition
unsigned int getValidAccountNum(const char *prompt);

int main(int argc, char *argv[])
{
    struct clientData clients[MAX_RECORDS] = {0}; // Primary memory store
    const char *filename = "clients.dat";
    FILE *cfPtr;
    unsigned int choice; // user's choice

    // Refactored for Speed: Load entire file into memory ONCE at startup
    if ((cfPtr = fopen(filename, "rb")) != NULL)
    {
        fread(clients, sizeof(struct clientData), MAX_RECORDS, cfPtr);
        fclose(cfPtr);
    }
    // If file doesn't exist, we start with the zero-initialized array 
    // and it will be created on exit.

    // enable user to specify action
    while ((choice = enterChoice()) != 8)
    {
        switch (choice)
        {
        case 1:
            textFile(clients);
            break;
        case 2:
            updateRecord(clients);
            break;
        case 3:
            newRecord(clients);
            break;
        case 4:
            deleteRecord(clients);
            break;
        case 5:
            listRecords(clients);
            break;
        case 6:
            searchRecordByName(clients);
            break;
        case 7:
            showTotalBalance(clients);
            break;
        default:
            puts("Incorrect choice");
            break;
        } // end switch
    }     // end while

    // Refactored for Speed: Save all changes back to disk ONCE on exit
    saveAndExit(clients, filename);

    return 0;
} // end main

// create formatted text file for printing
void textFile(const struct clientData clients[])
{
    FILE *writePtr; // accounts.txt file pointer
    int i;

    if ((writePtr = fopen("accounts.txt", "w")) == NULL)
    {
        puts("File could not be opened.");
    } 
    else
    {
        fprintf(writePtr, "%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");

        for (i = 0; i < MAX_RECORDS; i++)
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
void updateRecord(struct clientData clients[])
{
    unsigned int account = getValidAccountNum("Enter account to update ( 1 - 100 ): ");
    int index;
    double transaction;

    if (account == 0) return; // Invalid or aborted

    index = account - 1; // Map to array index

    // display error if account does not exist
    if (clients[index].acctNum == 0)
    {
        printf("Account #%u has no information.\n", account);
    }
    else
    { 
        printf("%-6u%-16s%-11s%10.2f\n\n", clients[index].acctNum, clients[index].lastName, clients[index].firstName, clients[index].balance);

        printf("%s", "Enter charge ( + ) or payment ( - ): ");
        if (scanf("%lf", &transaction) != 1) {
            int c; 
            while ((c = getchar()) != '\n' && c != EOF);
            puts("Invalid transaction amount.");
            return;
        }
        
        clients[index].balance += transaction; // update record balance purely in memory

        printf("%-6u%-16s%-11s%10.2f\n", clients[index].acctNum, clients[index].lastName, clients[index].firstName, clients[index].balance);
        puts("Account updated successfully.");
    } 
} // end function updateRecord

// delete an existing record
void deleteRecord(struct clientData clients[])
{
    unsigned int accountNum = getValidAccountNum("Enter account number to delete ( 1 - 100 ): ");
    int index;

    if (accountNum == 0) return;

    index = accountNum - 1;

    // display error if record does not exist
    if (clients[index].acctNum == 0)
    {
        printf("Account %u does not exist.\n", accountNum);
    } 
    else
    { 
        // Zero-out the struct in memory
        memset(&clients[index], 0, sizeof(struct clientData));
        printf("Account %u deleted successfully.\n", accountNum);
    } 
} // end function deleteRecord

// create and insert record
void newRecord(struct clientData clients[])
{
    unsigned int accountNum = getValidAccountNum("Enter new account number ( 1 - 100 ): ");
    int index;

    if (accountNum == 0) return;

    index = accountNum - 1;

    // display error if account already exists
    if (clients[index].acctNum != 0)
    {
        printf("Account #%u already contains information.\n", clients[index].acctNum);
    } 
    else
    { 
        printf("%s", "Enter lastname, firstname, balance\n? ");
        if (scanf("%14s%9s%lf", clients[index].lastName, clients[index].firstName, &clients[index].balance) != 3) {
            int c; 
            while ((c = getchar()) != '\n' && c != EOF);
            puts("Invalid input format.");
            // Reset to 0 since input failed
            memset(&clients[index], 0, sizeof(struct clientData));
            return;
        }

        clients[index].acctNum = accountNum;
        printf("Account %u added successfully.\n", accountNum);
    } 
} // end function newRecord

// list all accounts
void listRecords(const struct clientData clients[])
{
    int i;
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");

    for (i = 0; i < MAX_RECORDS; i++)
    {
        if (clients[i].acctNum != 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n", clients[i].acctNum, clients[i].lastName, clients[i].firstName, clients[i].balance);
        }
    }
    printf("---------------------------------------------\n\n");
} // end function listRecords

// search account by last name
void searchRecordByName(const struct clientData clients[])
{
    char searchName[15];
    int found = 0;
    int i;

    printf("Enter last name to search: ");
    if (scanf("%14s", searchName) != 1) {
        int c; 
        while ((c = getchar()) != '\n' && c != EOF);
        puts("Invalid input.");
        return;
    }
    
    printf("\n%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");
    for (i = 0; i < MAX_RECORDS; i++)
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
void showTotalBalance(const struct clientData clients[])
{
    double total = 0.0;
    int i;
    
    for (i = 0; i < MAX_RECORDS; i++)
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

// save the memory array back to binary file before terminating
void saveAndExit(const struct clientData clients[], const char *filename)
{
    FILE *writePtr;
    if ((writePtr = fopen(filename, "wb")) == NULL)
    {
        puts("Error: Could not save data to file upon exiting.");
    }
    else
    {
        fwrite(clients, sizeof(struct clientData), MAX_RECORDS, writePtr);
        fclose(writePtr);
        puts("Data successfully saved to file. Goodbye!");
    }
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