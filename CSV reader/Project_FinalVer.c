#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>  // For tolower and isdigit()

#define MAX_LINE_LENGTH 50
#define ROOM_NAME 50                                   // Also Helps prevent buffer overflow by handling incorrect or unexpected user input
#define MAX_TEMPS 30                                   // fulfills 0-30 validation and improves readability of constant rule values
#define MAX_ROOMS 100                                  // sets max unique room name limit
#define MAX_ROWS 100

typedef struct {
    char originalRoomName[ROOM_NAME];
    char normalizedRoomName[ROOM_NAME];
    float temperatures[MAX_TEMPS];
    int temperatureCount;
} RoomData;

void normalizeString(const char* str, char* normalizedStr) {
    int i = 0, j = 0;
    while (str[i] != '\0') {
        if (isalpha(str[i])) {
            normalizedStr[j++] = tolower(str[i]);
        }
        i++;
    }
    normalizedStr[j] = '\0';
}

int readCSV(const char *filename, RoomData rooms[], int *roomCount, int *rowCount) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("File not found.\n");
        return 1;
    }

    char buffer_line[MAX_LINE_LENGTH];
    *rowCount = 0;  // Starts count from 1 for accurate MAX_ROWS validation.

    if (fgets(buffer_line, sizeof(buffer_line), file) == NULL || fgets(buffer_line, sizeof(buffer_line), file) == NULL) {     // first reads header then the second checks if rows are empty or not
        printf("\nError: CSV file is empty or contains only the header with no data rows.\n");
        fclose(file);
        return 1;
    }

    // Process actual data rows
    while (fgets(buffer_line, sizeof(buffer_line), file)) {
        if (buffer_line[strlen(buffer_line) - 1] != '\n' && !feof(file)) {
            printf("\nError: line/s in file exceeds maximum allowed length of %d characters.\n", MAX_LINE_LENGTH);

            int ch;
            while ((ch = fgetc(file)) != '\n' && ch != EOF) {
                // Skipping the rest of the line
            }
            fclose(file);
            return 1;
        }

        // Sanitize buffer_line
        buffer_line[strcspn(buffer_line, "\r\n")] = 0;

        float temperature;
        char roomName[ROOM_NAME] = {0};  // setting to 0 avoids garbage buffer values appearing in the program strings

        // Checks if correctly reads both temperature and room name
        int parsedCount = sscanf(buffer_line, "%f,%49[^\n]", &temperature, roomName);

        if (parsedCount != 2) {
            printf("\nError: Incomplete data in CSV file. Expected 'temperature,room_name'.\n");
            printf("       %-25s : %s\n", "Data in question", buffer_line);  // Formatted printing for incomplete data error
            fclose(file);
            return 1;
        }

        char normalizedRoomName[ROOM_NAME] = {0};  // Zero-initialize normalizedRoomName
        normalizeString(roomName, normalizedRoomName);
        if (*rowCount >= MAX_ROWS) {
            printf("\nError: Max number of temp rows for each room exceeded %d limit.\n", MAX_ROWS);
            fclose(file);
            return 1;
        }

        int roomIndex = -1;
        for (int i = 0; i < *roomCount; i++) {
            if (strcmp(rooms[i].normalizedRoomName, normalizedRoomName) == 0) {
                roomIndex = i;
                break;
            }
        }

        if (roomIndex == -1) {
            if (*roomCount >= MAX_ROOMS) {
                printf("\nError: Maximum number of rooms exceeded.\n");
                fclose(file);
                return 1;
            }

            roomIndex = *roomCount;
            memset(rooms[roomIndex].originalRoomName, 0, ROOM_NAME);
            memset(rooms[roomIndex].normalizedRoomName, 0, ROOM_NAME);

            strcpy(rooms[roomIndex].originalRoomName, roomName);
            strcpy(rooms[roomIndex].normalizedRoomName, normalizedRoomName);
            rooms[roomIndex].temperatureCount = 0;
            (*roomCount)++;
        }

        rooms[roomIndex].temperatures[rooms[roomIndex].temperatureCount++] = temperature;
        (*rowCount)++;
    }

    fclose(file);
    return 0;
}

void printBarGraph(RoomData room) {
    printf("%s\n", room.originalRoomName);
    for (int i = 0; i < room.temperatureCount; i++) {
        float temp = room.temperatures[i];
        printf("%.1f", temp);

        if (temp < 0 || temp > MAX_TEMPS) {
            printf(" X\n");
        } else {
            for (int j = 0; j < (int)(temp * 2.0); j++) {
                printf("-");
            }
            printf("\n");
        }
    }
}

int isNumeric(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            return 0;  // If Not numeric
        }
    }
    return 1;  // If it's numeric
}

RoomData* findRoom(const char *typedRoomName, RoomData* roomsInCSV, int roomCount) {
    char normalizedRoom[ROOM_NAME] = {0};  // Zero-initialize normalizedRoom
    normalizeString(typedRoomName, normalizedRoom);

    for (int i = 0; i < roomCount; i++) {
        if (strcmp(roomsInCSV[i].normalizedRoomName, normalizedRoom) == 0) {
            return &roomsInCSV[i];
        }
    }
    return NULL;
}

void printAvailableRooms(RoomData rooms[], int roomCount) {
    printf("\nRoom not found. Compare your input to CSV room names:\n");
    printf("--------------------\n");    // Separator

    for (int i = 0; i < roomCount; i++) {
        printf("%-20s\n", rooms[i].originalRoomName);  // Formatted room name printing in column
    }
}

int main() {
    char typedRoomName[ROOM_NAME] = {0};  // Avoids garbage data upon function initialization
    RoomData rooms[MAX_ROOMS];
    int roomCount = 0;
    int rowCount = 0;

    // Initializes all room data to avoid garbage values
    for (int i = 0; i < MAX_ROOMS; i++) {
        memset(rooms[i].originalRoomName, 0, ROOM_NAME);
        memset(rooms[i].normalizedRoomName, 0, ROOM_NAME);
        rooms[i].temperatureCount = 0;
    }

    if (readCSV("test.csv", rooms, &roomCount, &rowCount) != 0) {
        return 1; // Failure
    }

    printf("\nAll Available Rooms are:\n");
    printf("--------------------\n");
    for (int i = 0; i < roomCount; i++) {
        printf("%-20s\n", rooms[i].originalRoomName);  // Formatted room name printing in column
    }

    while (1) {
        printf("\nType a Room here (or type 'end' to quit): ");
        fgets(typedRoomName, ROOM_NAME, stdin);

        size_t newLinePos = strcspn(typedRoomName, "\n");
        typedRoomName[newLinePos] = 0;

        if (strcmp(typedRoomName, "end") == 0) {
            printf("Thanks Bye.\n");
            break;
        }

        RoomData* room = NULL;
        if (isNumeric(typedRoomName) || (room = findRoom(typedRoomName, rooms, roomCount)) == NULL) {
            printAvailableRooms(rooms, roomCount);
        } else {
            printBarGraph(*room);
        }
    }

    return 0;
}
