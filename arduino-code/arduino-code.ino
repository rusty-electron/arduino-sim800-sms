#include <SoftwareSerial.h>
#include <Regexp.h>

SoftwareSerial mySerial(9, 10);
char currentLine[256] = "";
int currentLineIndex = 0;
String sender;
bool nextLineIsMessage;

void setup()
{
   mySerial.begin(9600);       
   Serial.begin(9600);       
   delay(100);

   mySerial.println("AT+CMGF=1");
   mySerial.println("AT+CNMI=2,2,0,0,0");
}

void loop(){
   if(mySerial.available()){
    char lastCharRead = mySerial.read();
    
    if(lastCharRead == '\r' || lastCharRead == '\n'){
        String lastLine = String(currentLine);
        if(lastLine.startsWith("+CMT:")){

          // Extract Sender Phone Number
          char number[13];
          MatchState ns;
          ns.Target(currentLine);
          
          ns.Match("\+91%d+");

          Serial.println("\n" + lastLine);
          sender = ns.GetMatch(number);
          
          Serial.println(sender);
          nextLineIsMessage = true;
           
        } else if (lastLine.length() > 0) {
         
          if(nextLineIsMessage) {
            Serial.println(lastLine);

            // Extract Validation Key (if any)
            char code[7];
            MatchState key;
            key.Target(currentLine);
            key.Match("%a%d%a%a%a%a");
            
            Serial.println("Matched ID: " + (String)key.GetMatch(code));  
            if(lastLine.indexOf("You have received") >= 0 && (key.MatchLength == 6)){
              
               // Extract Credit Value
               char credit[20];
               MatchState mo;
               mo.Target(currentLine);
               mo.Match("Rs. %d+\.?%d*");
               
               Serial.println(mo.MatchLength);
               mo.GetMatch(credit);
               
               if(mo.MatchLength <= sizeof(credit)){
                  float price = atof(&credit[4]);
                  Serial.print("Credit Amount: ");
                  Serial.println(price);

                  sendMessage();
               } else {
                  Serial.println("Credit String: Match not found");
               }
            }
             
            nextLineIsMessage = false;
          }
           
        }
        //Clear char array for next line of read
        for( int i = 0; i < sizeof(currentLine);  ++i ) {
         currentLine[i] = (char)0;
        }
        currentLineIndex = 0;
    } else {
      currentLine[currentLineIndex++] = lastCharRead;
    }
  }
}

void sendMessage(){
   mySerial.println("AT+CMGS=\""+ sender +"\"\r"); // Replace x with mobile number
   delay(1000);
   mySerial.println("Transaction Successful");// The SMS text you want to send
   delay(100);
   mySerial.println((char)26);// ASCII code of CTRL+Z
   delay(1000);

   mySerial.println("AT+CNMI=2,2,0,0,0");
   delay(500);
}

