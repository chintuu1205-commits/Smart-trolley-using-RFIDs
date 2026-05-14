#include <SPI.h> 
#include <MFRC522.h> 
#include <LiquidCrystal.h> 
#include <SoftwareSerial.h> 
 
// ---------- RFID Pins ---------- 
#define SS_PIN 10 
#define RST_PIN 9 
MFRC522 rfid(SS_PIN, RST_PIN);  
// ---------- LCD ---------- 
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5); 
 
// ---------- Switches ---------- 
#define TOTAL_SWITCH 8 
#define INCREMENT_SWITCH 3 
#define DECREMENT_SWITCH 4 
#define BUZZER_PIN 5 
 
// ---------- Barcode Scanner ---------- 
#define BARCODE_RX 7 
#define BARCODE_TX 6 
SoftwareSerial myScanner(BARCODE_RX, BARCODE_TX); 
 
 // ---------- RFID Database ---------- const byte monUIDs[][5] = { 
  { 35, 85, 197, 44 }, 
  { 36, 183, 201, 113 }, 
  { 36, 248, 16, 117}, 
  { 196, 79, 68, 117}, 
  { 68, 205, 166, 114} 
}; 
const char* cardNames[] = {"milk","wheat","rice","oil","salt"}; int prices[] = {30,100,60,75,40}; int quantities[5] = {0,0,0,0,0}; 
 
// ---------- Barcode Database ---------- 
String barcodes[] = {"000602","000600","000601","000603","000603"}; String barcodeNames[] = {"Item1","Item2","Item3","Item4","Item5"}; int barcodePrices[] = {30,75,50,60,40}; int barcodeQty[5] = {0,0,0,0,0}; 
 
// ---------- Uno string ---------- 
String uno; 
 
void setup() {   Serial.begin(9600);   myScanner.begin(9600);   SPI.begin();   rfid.PCD_Init(); 
 
  pinMode(TOTAL_SWITCH, INPUT_PULLUP);   pinMode(INCREMENT_SWITCH, INPUT_PULLUP);   pinMode(DECREMENT_SWITCH, INPUT_PULLUP);   pinMode(BUZZER_PIN, OUTPUT); 
 
  lcd.begin(16,2);   lcd.clear();   lcd.print("Welcome to");   lcd.setCursor(0,1);   lcd.print("YMTS mart");   buzz(500); 
delay(2000); lcd.clear(); 
} 
 
void loop() {   lcd.clear();   lcd.print("Scan items..."); 
 
  // ----- RFID Section ----- 
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()){     for (int i=0;i<rfid.uid.size;i++){ 
      Serial.print(rfid.uid.uidByte[i],HEX); Serial.print(" "); 
    } 
    Serial.println(); 
 
    for (int j=0;j<5;j++){ 
      if (compareUID(rfid.uid.uidByte, monUIDs[j])){         showItem(j, true);  // Display item first         adjustQuantity(j, true); // Then increment/decrement         break; 
      } 
    } 
    rfid.PICC_HaltA(); 
  } 
 
  // ----- Barcode Section -----   if (myScanner.available()){ 
    String code = myScanner.readStringUntil('\n');     code.trim(); 
    Serial.println("Barcode: "+code); 
 
    int index = getBarcodeIndex(code);     if(index != -1){ 
      showItem(index, false);   // Display item first 
      adjustQuantity(index, false); // Then increment/decrement 
    } 
  } 
 
  // ----- Total ----- 
  if(digitalRead(TOTAL_SWITCH) == LOW){ 
    displayTotal(); 
  } 
} 
 
// ---------- Functions ---------- 
 
bool compareUID(byte* uid1,const byte* uid2){   for(int i=0;i<4;i++){     if(uid1[i]!=uid2[i]) return false; 
  }   return true; 
} 
 
// Show item cost on LCD immediately after scan void showItem(int index, bool isRFID){   lcd.clear(); 
  if(isRFID) lcd.print(cardNames[index]);   else lcd.print(barcodeNames[index]);   lcd.setCursor(0,1);   lcd.print("Rs: "); 
  lcd.print(isRFID ? prices[index] : barcodePrices[index]);   buzz(200);   delay(1000); 
} 
 
// Wait and adjust quantity based on INCREMENT/DECREMENT void adjustQuantity(int index, bool isRFID){   bool pressed=false; 
unsigned long start=millis(); 
while(!pressed && millis()-start<5000){  // Wait 5 sec for user input 
    if(digitalRead(INCREMENT_SWITCH)==LOW){ 
      if(isRFID) quantities[index]++;       else barcodeQty[index]++;       pressed=true;       buzz(300); 
    } 
    if(digitalRead(DECREMENT_SWITCH)==LOW){ 
      if(isRFID) quantities[index] = max(0, quantities[index]-1);       else barcodeQty[index] = max(0, barcodeQty[index]-1);       pressed=true;       buzz(300); 
    } 
  } 
  // Default increment if no switch pressed   if(!pressed){ 
    if(isRFID) quantities[index]++;     else barcodeQty[index]++; 
  } 
  printItem(index,isRFID); 
} 
 
// Print item with current quantity and cost void printItem(int index, bool isRFID){   lcd.clear(); 
  if(isRFID) lcd.print(cardNames[index]);   else lcd.print(barcodeNames[index]);   lcd.setCursor(0,1);   lcd.print("Qty: "); 
  lcd.print(isRFID ? quantities[index] : barcodeQty[index]);   lcd.print(" Rs: ");   lcd.print(isRFID ? quantities[index]*prices[index] : 
barcodeQty[index]*barcodePrices[index]); 
delay(1500); lcd.clear(); 
} 
 
// Get barcode index int getBarcodeIndex(String code){   for(int i=0;i<5;i++){     if(code==barcodes[i]) return i; 
  }   return -1; 
} 
 
// Buzz helper void buzz(int duration){   digitalWrite(BUZZER_PIN,HIGH);   delay(duration);   digitalWrite(BUZZER_PIN,LOW); 
} 
 
// Display total and print uno void displayTotal(){   int totalCost=0; 
 
  for(int i=0;i<5;i++){     totalCost += quantities[i]*prices[i];     totalCost += barcodeQty[i]*barcodePrices[i]; 
  } 
 
  lcd.clear();   lcd.print("Grand Total:");   lcd.setCursor(0,1);   lcd.print("Rs ");   lcd.print(totalCost); 
 
// --- Uno string --- 
uno = String("a") + String(totalCost) + String("b"); 
  Serial.println(uno); 
  Serial.print("@"); 
  Serial.println(totalCost); 
 
  buzz(500);   delay(2000);   lcd.clear(); 
}