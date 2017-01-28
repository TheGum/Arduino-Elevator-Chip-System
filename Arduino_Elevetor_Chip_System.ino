// Thanks to https://github.com/firebull/arduino-edb/ for the Library and SD CARD example.

#include "Arduino.h"
#include <EDB.h>

// Use the external SPI SD card as storage
#include <SPI.h>
#include <SD.h>

#define SD_PIN 4  // SD Card CS pin according to SD Card type or Board Shield
#define TABLE_SIZE 8192


// <--------------------------------- SD Card Database -----------------------------------> \\

char* db_name = "/db/Peoples.db";
File dbFile;

// Structure for all data
struct LogEvent {
  int id;
  char* username;
  int apartment;
  char* cardId;
  bool cardStatus;
}
logEvent;

// The write handler for using the SD Library
void writer (unsigned long address, byte data) {
  dbFile.seek(address);
  dbFile.write(data);
  dbFile.flush();
}

// The read handler for using the SD Library
byte reader (unsigned long address) {
  dbFile.seek(address);
  byte b = dbFile.read();
  return b;
}

// EDB object with write and read handlers
EDB db(&writer, &reader);

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  Serial.begin(9600);
  Serial.println("Start Project:");
  Serial.println();

  randomSeed(analogRead(0));

  databaseConfiguration();
}

void databaseConfiguration() {
  if (!SD.begin(SD_PIN)) {
    Serial.println("No SD-card.");
    return;
  }

  // Check dir for db files
  if (!SD.exists("/db")) {
    Serial.println("DB directory does not exist, creating...");
    SD.mkdir("/db");
  }

  if (SD.exists(db_name)) {

    dbFile = SD.open(db_name, FILE_WRITE);

    // Sometimes it wont open at first attempt, espessialy after cold start
    if (!dbFile) {
      dbFile = SD.open(db_name, FILE_WRITE);
    }

    if (dbFile) {
      Serial.print("Openning current table... ");
      EDB_Status result = db.open(0);
      if (result == EDB_OK) {
        Serial.println("DONE");
      } else {
        Serial.println("ERROR");
        Serial.println("Did not find database in the file " + String(db_name));
        Serial.print("Creating new table... ");
        db.create(0, TABLE_SIZE, (unsigned int)sizeof(logEvent));
        Serial.println("DONE");
        return;
      }
    } else {
      Serial.println("Could not open file " + String(db_name));
      return;
    }
  } else {
    Serial.print("Creating table... ");
    // create table at with starting address 0
    dbFile = SD.open(db_name, FILE_WRITE);
    db.create(0, TABLE_SIZE, (unsigned int)sizeof(logEvent));
    Serial.println("DONE");
  }

  databaseAppendAllRecords();

  // dbFile.close();
}

void databaseAppendAllRecords() {
  deleteAll();

  selectAll();
}

void selectOneRecord(int Id)
{
  db.readRec(Id, EDB_REC logEvent);

  Serial.print("Id: ");
  Serial.print(Id);
  Serial.print(", Username: ");
  Serial.print(logEvent.username);
  Serial.print(", Apartment: ");
  Serial.print(logEvent.apartment);
  Serial.print(", CardID: ");
  Serial.print(logEvent.cardId);
  Serial.print(", Status: ");
  Serial.println(logEvent.cardStatus);
}

void selectAll()
{
  for (int Id = 1; Id <= db.count(); Id++)
  {
    db.readRec(Id, EDB_REC logEvent);

    Serial.print("Id: ");
    Serial.print(Id);
    Serial.print(", Username: ");
    Serial.print(logEvent.username);
    Serial.print(", Apartment: ");
    Serial.print(logEvent.apartment);
    Serial.print(", CardID: ");
    Serial.print(logEvent.cardId);
    Serial.print(", Status: ");
    if (logEvent.cardStatus == true) Serial.println("Active");
    else Serial.println("Ban");

  }
}

void updateOneRecord(int Id, bool CardStatus)
{
  Serial.print("Updating record with Id: ");
  Serial.print(Id);
  Serial.print("... ");
  logEvent.id = Id;
  logEvent.cardStatus = CardStatus;
  EDB_Status result = db.updateRec(Id, EDB_REC logEvent);
  if (result != EDB_OK) printError(result);
  Serial.println("DONE");
}

void appendOneRecord(char* Username, int Apartment, char* CardID, bool CardStatus)
{
  Serial.print("Appending record... ");

  logEvent.id = countRecords + 1;
  logEvent.username = Username;
  logEvent.apartment = Apartment;
  logEvent.cardId = CardID;
  logEvent.cardStatus = CardStatus;

  EDB_Status result = db.appendRec(EDB_REC logEvent);
  if (result != EDB_OK) printError(result);

  Serial.println("DONE");
}

void recordLimit()
{
  Serial.print("Record Limit: ");
  Serial.println(db.limit());
}

void deleteOneRecord(int Id)
{
  Serial.print("Deleting user with Id: ");
  Serial.println(Id);
  db.deleteRec(Id);
}

void deleteAll()
{
  Serial.print("Delete all users... ");
  db.clear();
  Serial.println("DONE");
}

void countRecords()
{
  Serial.print("Users count: ");
  Serial.println(db.count());
}

void printError(EDB_Status err)
{
  Serial.print("ERROR: ");
  switch (err)
  {
    case EDB_OUT_OF_RANGE:
      Serial.println("Recno out of range");
      break;
    case EDB_TABLE_FULL:
      Serial.println("Table full");
      break;
    case EDB_OK:
    default:
      Serial.println("OK");
      break;
  }
}

// <-----------------------------------   RFID   -----------------------------------------> \\

void loop()
{
}

bool CheckCardStatus(char* CardId) { 
  for (int Id = 1; Id <= db.count(); Id++) {
    db.readRec(Id, EDB_REC logEvent);

    if (logEvent.cardId == CardId) return logEvent.cardStatus;
  }
}



