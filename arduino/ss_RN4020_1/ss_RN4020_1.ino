/* 
  SoftwareSerial, RN4020 BLE
 *  v 0.9.2
*/
#include <SoftwareSerial.h>
SoftwareSerial mySerial(5, 6); /* RX:D5, TX:D6 */
String mBuff="";
const int mVoutPin = 0;
const int mOK_CODE=1;
const int mNG_CODE=0;
uint32_t mTimer=0;
const int mNextSec= 30; //Sec
/*
 value: check string
 maxMsec : max Wait (mSec)
*/
/*
int Is_validResult(String value, int maxMsec ){
  int ret= mNG_CODE;
  uint32_t tm_maxWait=millis();
  int iLen= value.length();
  String sBuff="";
  while( mySerial.available() ){
    char c= mySerial.read();
Serial.print( c );
    sBuff.concat(c );
  }
  Serial.println("");
  if(sBuff.length() < 1){ return ret; }
    int iLenBuff= sBuff.length();
    Serial.println("iLenBuff="+ String(iLenBuff)) ;
    int iSt=iLen+ 2; 
    String sRecv = sBuff.substring((iLenBuff -iSt ) , iLen );
    Serial.println( "sBuff="+ sBuff+ " ,sRecv="+ sRecv );          
    if(sRecv == value ){
      ret= mOK_CODE;
    }
  return ret;  
}
*/
//
long convert_Map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//
// reading LM60BIZ
int getTempNum(){
  int iRet=0;
//  float fSen  = 0;
  unsigned long reading  = 0;   
  for (int i=0; i<10; i++) {
    int  iTmp = analogRead(mVoutPin);
//Serial.print("get.iTmp=");
//Serial.println(iTmp);
    reading  += iTmp; 
    delay(100);
  }
  int SValue= reading / 10;
  int voltage=convert_Map(SValue, 0, 1000, 0,3300);  // V
Serial.println("SValue="+ String(SValue)   +" , voltage="  +String(voltage ) );
  int iTemp = (voltage - 424) / 6.25; //電圧値を温度に変換, offset=425
  iRet= iTemp;
  
  return iRet;  
}
//
int Is_resWait(String value, uint32_t maxMsec ){
  int ret= mNG_CODE;
  
  uint32_t tm_maxWait=millis();
  int iLen= value.length();
  String sBuff="";
  int iBool=1;
    while(iBool ){
        while( mySerial.available() ){
            char c= mySerial.read();
            Serial.print( c );
            if( (c != 0x0a ) && (c != 0x0d ) ){
                sBuff.concat(c );
            }
            
        } //end_while
        if(  (int )sBuff.length() >= iLen ){ iBool=0;  }      
        delay(100);
        uint32_t tmMax =millis() -tm_maxWait;
        if(tmMax > (uint32_t)maxMsec ){ 
          Serial.println("#Error-Is_resWait:maxSec!");
          iBool=0; 
        } 
    }//end_while_1
    Serial.println("");
    if(sBuff.length() < 1){ return ret; }
    int iLenBuff= sBuff.length();
    //int iSt=iLenBuff -(iLen+ 2);
    int iSt=iLenBuff -(iLen); 
    Serial.println("iLenBuff="+ String(iLenBuff)+",iSt="+ String(iSt) ) ;
    String sRecv = sBuff.substring(iSt   , iLen );
    Serial.println( "sBuff="+ sBuff+ " ,sRecv="+ sRecv );
    if(sRecv == value ){
      ret= mOK_CODE;
    }
  return ret;  
}
//
void proc_getSSerial(){
  while( mySerial.available() ){
    char c= mySerial.read();
Serial.print( c );
  } //end_while
}

int mCounter=0;
//
void proc_sendCmd(){
  int iWaitTm= 5000;
  char cBuff[24 +1];
  int iSenNum= getTempNum();
  Serial.println("iSenNum=" + String(iSenNum) );
  sprintf(cBuff , "SN,D12%06d000001\r", iSenNum );
  //SN
  Serial.println( "#Start SN, cBuff="+  String(cBuff)  );
   mySerial.print(String(cBuff) );
   if( Is_resWait("AOK", iWaitTm ) == mNG_CODE ){ return; }
   else{Serial.println("#OK SN" ); };
   proc_getSSerial();
//   wait_forSec(3);
   //R
   Serial.println( "#Start R" );
   mySerial.print("R,1\r");
   delay(1000);
   if( Is_resWait("RebootCMD", iWaitTm ) == mNG_CODE ){ return; }
   else{Serial.println("#OK R1" ); };
   proc_getSSerial();
   //wait_forSec(3);
   
   //A
   Serial.println( "#Start A" );
//   mySerial.print("A\r");
   mySerial.print("A,0064,07D0\r");    //100msec, total= 2000mSec
  if( Is_resWait("AOK", iWaitTm ) == mNG_CODE ){ return; }
   else{Serial.println("#OK A" ); };
   proc_getSSerial();
   mCounter= mCounter+1;
   mTimer = millis() + (mNextSec * 1000);
}

//
void wait_forSec(int wait){
    for(int i=0; i<wait; i++){
    delay(1000);
    Serial.println("#wait_forMsec: "+ String(i) );    
  }
}
//
void setup() {
  // 115200
  Serial.begin( 9600 );
  mySerial.begin( 9600 );
  Serial.println("#Start-setup-SS");
  //wait
  wait_forSec(3);
  proc_getSSerial(); //cear-buff
  proc_sendCmd();
  Serial.println( "mTimer="+ String(mTimer) );  
}

//
void loop() {
//  delay(100);
//  Serial.println( "mTimer="+ String(mTimer) + ",millis=" + String(millis()) );
  if(mTimer < millis() ){
      mTimer = millis() + (mNextSec * 1000);
      proc_sendCmd();
  }

}

