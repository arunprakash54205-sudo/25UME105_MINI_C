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
void displaySortedByBalance(const struct clientData clients[]);
void exportToCSV(const struct clientData clients[]);
void transferFunds(struct clientData clients[]);
void countActiveAccounts(const struct clientData clients[]);
void applyInterest(struct clientData clients[]);
void findOverdrawnAccounts(const struct clientData clients[]);
void factoryReset(struct clientData clients[]);
void backupData(const struct clientData clients[]);
void restoreData(struct clientData clients[]);
void saveAndExit(const struct clientData clients[], const char *filename);

// Helper function prototypes for functional decomposition
unsigned int getValidAccountNum(const char *prompt);
int compareBalance(const void *a, const void *b);

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

    // enable user to specify action
    while ((choice = enterChoice()) != 17)
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
        case 8:
            displaySortedByBalance(clients);
            break;
        case 9:
            exportToCSV(clients);
            break;
        case 10:
            transferFunds(clients);
            break;
        case 11:
            countActiveAccounts(clients);
            break;
        case 12:
            applyInterest(clients);
            break;
        case 13:
            findOverdrawnAccounts(clients);
            break;
        case 14:
            factoryReset(clients);
            break;
        case 15:
            backupData(clients);
            break;
        case 16:
            restoreData(clients);
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

// export to CSV format for external processing
void exportToCSV(const struct clientData clients[])
{
    FILE *writePtr;
    int i;

    if ((writePtr = fopen("accounts.csv", "w")) == NULL)
    {
        puts("File could not be opened.");
    }
    else
    {
        fprintf(writePtr, "Account,LastName,FirstName,Balance\n");
        for (i = 0; i < MAX_RECORDS; i++)
        {
            if (clients[i].acctNum != 0)
            {
                fprintf(writePtr, "%u,%s,%s,%.2f\n", clients[i].acctNum, clients[i].lastName, clients[i].firstName, clients[i].balance);
            }
        }
        fclose(writePtr);
        puts("Successfully exported accounts to accounts.csv.");
    }
}

// compare function for sorting accounts by balance (descending)
int compareBalance(const void *a, const void *b)
{
    const struct clientData *clientA = (const struct clientData *)a;
    const struct clientData *clientB = (const struct clientData *)b;
    
    // Sort descending (highest balance first)
    if (clientA->acctNum == 0 && clientB->acctNum == 0) return 0;
    if (clientA->acctNum == 0) return 1;
    if (clientB->acctNum == 0) return -1;

    if (clientB->balance > clientA->balance) return 1;
    if (clientB->balance < clientA->balance) return -1;
    return 0;
}

// display all active accounts sorted by highest balance
void displaySortedByBalance(const struct clientData clients[])
{
    struct clientData sortedClients[MAX_RECORDS];
    int i;
    
    // Copy active clients to temporary array
    memcpy(sortedClients, clients, sizeof(struct clientData) * MAX_RECORDS);
    
    // Sort the copy using qsort
    qsort(sortedClients, MAX_RECORDS, sizeof(struct clientData), compareBalance);
    
    printf("\n--- Accounts Sorted by Balance (Highest to Lowest) ---\n");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");

    for (i = 0; i < MAX_RECORDS; i++)
    {
        if (sortedClients[i].acctNum != 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n", sortedClients[i].acctNum, sortedClients[i].lastName, sortedClients[i].firstName, sortedClients[i].balance);
        }
    }
    printf("---------------------------------------------\n\n");
}

// update balance in record
void updateRecord(struct clientData clients[])
{
    unsigned int account;
    int index;
    double transaction;
    int c;

    account = getValidAccountNum("Enter account to update ( 1 - 100 ): ");
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
            while ((c = getchar()) != '\n' && c != EOF);
            puts("Invalid transaction amount.");
            return;
        }
        
        clients[index].balance += transaction; 

        printf("%-6u%-16s%-11s%10.2f\n", clients[index].acctNum, clients[index].lastName, clients[index].firstName, clients[index].balance);
        puts("Account updated successfully.");
    } 
} // end function updateRecord

// transfer funds between two accounts
void transferFunds(struct clientData clients[])
{
    unsigned int fromAccount, toAccount;
    int fromIndex, toIndex;
    double amount;
    int c;

    fromAccount = getValidAccountNum("Enter account to transfer FROM ( 1 - 100 ): ");
    if (fromAccount == 0) return;
    
    toAccount = getValidAccountNum("Enter account to transfer TO ( 1 - 100 ): ");
    if (toAccount == 0) return;

    if (fromAccount == toAccount) {
        puts("Cannot transfer funds to the same account.");
        return;
    }

    fromIndex = fromAccount - 1;
    toIndex = toAccount - 1;

    if (clients[fromIndex].acctNum == 0) {
        printf("Sender account #%u does not exist.\n", fromAccount);
        return;
    }
    if (clients[toIndex].acctNum == 0) {
        printf("Receiver account #%u does not exist.\n", toAccount);
        return;
    }

    printf("Enter transfer amount: ");
    if (scanf("%lf", &amount) != 1) {
        while ((c = getchar()) != '\n' && c != EOF);
        puts("Invalid amount.");
        return;
    }

    if (amount < 0) {
        puts("Cannot transfer a negative amount.");
        return;
    }
    if (clients[fromIndex].balance < amount) {
        puts("Insufficient funds in sender account.");
        return;
    }

    clients[fromIndex].balance -= amount;
    clients[toIndex].balance += amount;

    puts("Transfer successful!");
    printf("New Balance for Sender Account #%u: %.2f\n", fromAccount, clients[fromIndex].balance);
    printf("New Balance for Receiver Account #%u: %.2f\n", toAccount, clients[toIndex].balance);
}

// delete an existing record
void deleteRecord(struct clientData clients[])
{
    unsigned int accountNum;
    int index;

    accountNum = getValidAccountNum("Enter account number to delete ( 1 - 100 ): ");
    if (accountNum == 0) return;

    index = accountNum - 1;

    if (clients[index].acctNum == 0)
    {
        printf("Account %u does not exist.\n", accountNum);
    } 
    else
    { 
        memset(&clients[index], 0, sizeof(struct clientData));
        printf("Account %u deleted successfully.\n", accountNum);
    } 
} // end function deleteRecord

// create and insert record
void newRecord(struct clientData clients[])
{
    unsigned int accountNum;
    int index;
    int c;

    accountNum = getValidAccountNum("Enter new account number ( 1 - 100 ): ");
    if (accountNum == 0) return;

    index = accountNum - 1;

    if (clients[index].acctNum != 0)
    {
        printf("Account #%u already contains information.\n", clients[index].acctNum);
    } 
    else
    { 
        printf("%s", "Enter lastname, firstname, balance\n? ");
        if (scanf("%14s%9s%lf", clients[index].lastName, clients[index].firstName, &clients[index].balance) != 3) {
            while ((c = getchar()) != '\n' && c != EOF);
            puts("Invalid input format.");
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
    int c;

    printf("Enter last name to search: ");
    if (scanf("%14s", searchName) != 1) {
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

// show total active accounts
void countActiveAccounts(const struct clientData clients[])
{
    int activeCount = 0;
    int i;
    for (i = 0; i < MAX_RECORDS; i++)
    {
        if (clients[i].acctNum != 0) activeCount++;
    }
    printf("\n---------------------------------------------\n");
    printf("Total Active Accounts: %d / %d\n", activeCount, MAX_RECORDS);
    printf("---------------------------------------------\n\n");
}

// apply interest to all accounts with positive balances
void applyInterest(struct clientData clients[])
{
    double rate;
    int c;
    int i;
    int count = 0;

    printf("Enter interest rate percentage (e.g., 5 for 5%%): ");
    if (scanf("%lf", &rate) != 1) {
        while ((c = getchar()) != '\n' && c != EOF);
        puts("Invalid rate.");
        return;
    }

    for (i = 0; i < MAX_RECORDS; i++)
    {
        if (clients[i].acctNum != 0 && clients[i].balance > 0)
        {
            clients[i].balance += clients[i].balance * (rate / 100.0);
            count++;
        }
    }
    printf("Successfully applied %.2f%% interest to %d accounts.\n", rate, count);
}

// find all overdrawn (negative balance) accounts
void findOverdrawnAccounts(const struct clientData clients[])
{
    int i;
    int found = 0;

    printf("\n--- Overdrawn Accounts (Negative Balance) ---\n");
    printf("%-6s%-16s%-11s%10s\n", "Acct", "Last Name", "First Name", "Balance");
    printf("---------------------------------------------\n");

    for (i = 0; i < MAX_RECORDS; i++)
    {
        if (clients[i].acctNum != 0 && clients[i].balance < 0)
        {
            printf("%-6u%-16s%-11s%10.2f\n", clients[i].acctNum, clients[i].lastName, clients[i].firstName, clients[i].balance);
            found++;
        }
    }

    if (found == 0) {
        puts("No overdrawn accounts found. Great!");
    }
    printf("---------------------------------------------\n\n");
}

// wipe all accounts (factory reset)
void factoryReset(struct clientData clients[])
{
    int c;
    char confirm;
    
    printf("\nWARNING: This will permanently delete ALL accounts and balances!\n");
    printf("Are you sure you want to proceed? (y/n): ");
    if (scanf(" %c", &confirm) != 1) {
        while ((c = getchar()) != '\n' && c != EOF);
        puts("Invalid input.");
        return;
    }
    
    if (confirm == 'y' || confirm == 'Y') {
        memset(clients, 0, sizeof(struct clientData) * MAX_RECORDS);
        puts("\nFACTORY RESET COMPLETE. All data has been wiped.");
    } else {
        puts("\nFactory reset aborted. Data is safe.");
    }
}

// create a full backup of all accounts
void backupData(const struct clientData clients[])
{
    FILE *writePtr;
    if ((writePtr = fopen("clients_backup.dat", "wb")) == NULL)
    {
        puts("Error: Could not create backup file.");
    }
    else
    {
        fwrite(clients, sizeof(struct clientData), MAX_RECORDS, writePtr);
        fclose(writePtr);
        puts("\nSuccessfully backed up all accounts to clients_backup.dat!\n");
    }
}

// restore all accounts from the backup
void restoreData(struct clientData clients[])
{
    FILE *readPtr;
    int c;
    char confirm;
    
    printf("\nWARNING: This will overwrite current memory with the backup data.\n");
    printf("Are you sure you want to proceed? (y/n): ");
    if (scanf(" %c", &confirm) != 1) {
        while ((c = getchar()) != '\n' && c != EOF);
        puts("Invalid input.");
        return;
    }
    
    if (confirm == 'y' || confirm == 'Y') {
        if ((readPtr = fopen("clients_backup.dat", "rb")) != NULL)
        {
            fread(clients, sizeof(struct clientData), MAX_RECORDS, readPtr);
            fclose(readPtr);
            puts("\nSuccessfully restored data from clients_backup.dat!\n");
        }
        else
        {
            puts("\nError: No backup file found (clients_backup.dat).\n");
        }
    } else {
        puts("\nRestore aborted. Current data is intact.\n");
    }
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
    int c;
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
                 "8 - display accounts sorted by balance\n"
                 "9 - export accounts to CSV\n"
                 "10 - transfer funds between accounts\n"
                 "11 - count active accounts\n"
                 "12 - apply interest to all positive balances\n"
                 "13 - find overdrawn accounts\n"
                 "14 - clear all accounts (Factory Reset)\n"
                 "15 - backup data to secure file\n"
                 "16 - restore data from backup\n"
                 "17 - end program\n? ");

    if (scanf("%u", &menuChoice) != 1) {
        // clear input buffer if invalid entry is made
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
    int c;

    printf("%s", prompt);
    if (scanf("%u", &accountNum) != 1)
    {
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