#include <ESP8266WiFi.h> //--> khai báo thư viện wifi, cả station mode và access point
#include <WiFiClient.h> //--> khai báo thư viện wificilent
#include <ESP8266WebServer.h> //--> khai báo thư viện webserver

//----------------------------------------

#include "index.h" // -->khai báo thêm trang đính kèm, đọc dữ liệu bên index.html, webpage dạng tĩnh

#define ON_Board_LED 2  // --> khởi tạo hằng số kết nối led trên mạch là chân D4 (GPIO2 = D4), được sử dụng để báo quá trình kết nối tới wifi.


//---------------------------------------- tên wifi và mật khẩu sẽ kết nối
const char* ssid = "utpLink";  //--> tên wifi được sử dụng, ở đây mình sẽ lấy chính lap thành ap( access point ) --> trạm phát sóng wifi.
const char* password = "12345678"; //--> mật khẩu wifi được sử dụng
//----------------------------------------

ESP8266WebServer server(80);  //--> Server trên port = 80

unsigned long previousMillisGetHR = 0; // khai báo biến previousMillisGetHR để lưu trữ lần cuối cùng nhịp tim đập
unsigned long previousMillisHR = 0; //khai báo biến previousMillisHR để lấy giá trị cuối cùng của BPM
const long intervalGetHR = 10; // khai báo biến intervalGetHR để đưa ra khoảng thời gian đọc nhịp tim đập = 10ms.  
const long intervalHR = 10000; // khai báo biến intervalHR = 10000 để đưa ra khoảng thời gian tính toán và lấy giá trị BPM thực tế, 10000 = 10s.

const int PulseSensorHRWire = A0; //khai báo hằng PulseSensorHRWire(cảm biến nhịp tim) là chân A0 trên bo mạch 
const int LED_D1 = D1; //khai báo hằng LED_D1 là chân D1 trên bo mạch, led này biểu thị nhịp tim đập / giây.
int Threshold = 600; //khai báo biến Thresold để lấy giá trị ngưỡng = 600, khi cảm biến nhịp tim đọc được giá trị trên 600 thì sẽ tính là nhịp, còn nếu dưới 600 thì sẽ không tính


int cntHB = 0; //-->  khai báo biến cntHB để đếm được số nhịp tim đập.
boolean ThresholdStat = true; //--> khai báo kiểu bool ThresholdStat = true để tính toán nhịp tim đập,
int BPMval = 0; //--> biến để giữ kết quả tính toán nhịp tim





//--------------------------------------------------------------------------------void handleRoot()
//  chương trình dưới được chạy khi IP kết nối của nodemcu esp12E được mở ra trên các trình duyệt web
void handleRoot() 
{
 String s = MAIN_page;
 server.send(200, "text/html", s  ); // khi IP kết nối của nodemcu esp12E được mở ra trên trình duyệt web thì esp8266 sẽ gửi dữ liệu lên máy khách và trình duyệt được mở sẽ đọc file Index.h với hằng được khai báo ở file là MAIN_page, với tốc độ là 200millis
}



//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------void handleHeartRate()
// chương trình dưới sẽ được chạy sẽ được chạy khi trình duyệt(máy khách) đã kết nối thành công đến máy chủ(bo esp8266)
// --> chương trình sẽ hiển thị ra giá trị BMP và gửi cho máy khách
void handleHeartRate() {
  digitalWrite(ON_Board_LED, LOW); // biến được khai báo từ đầu D4, khi kết nối được với máy khách và kết nối được với webpage "ON_Board_LED" sẽ đưa về  mức 0, bật LED.
  String BPMvalSend = String(BPMval); // khởi tạo ra chuỗi BMPvalSend = BMPval(biến giữ kết quả tính toán nhịp tim) --> giá trị gửi lên webpage = giá trị thực ở esp8266
  server.send(200, "text/plane", BPMvalSend);
  //gửi dữ liệu của máy chủ lên máy khách với dữ liệu được gửi là BMPvalSend(được khai báo ở dòng trên = biến đễ giữ kết quả tính toán nhịp tim)
  //text/plain --> gửi giá trị ADC với phương thức ajax, ADC --> analog to digital converted, đọc giá trị analog từ cổng A0 đã được khởi tạo và truyền đi với dữ liệu digital.
  //tốc độ truyền 200millis
  digitalWrite(ON_Board_LED, HIGH); // tắt LED bởi vì đã kết nối được với trình duyệt.
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------void GetHeartRate()

// chương trình con để đọc nhịp tim và tính toán để có giá trị BMP, nhận giá trị BMP dựa trên nhịp chỉ số nhịp tim sau 10 giây.
void GetHeartRate() {
 
  unsigned long currentMillisGetHR = millis(); //  khai báo biến currentMillisGetHR giá trị đọc nhịp tim hiện tại

  if (currentMillisGetHR - previousMillisGetHR >= intervalGetHR) {//nếu như giá trị đọc hiện tại - giá trị đọc lần cuối >= intervalGetHR(khoảng thời gian nhịp tim đập)
    previousMillisGetHR = currentMillisGetHR;// giá trị đọc lần cuối = giá trị đọc hiện tại

    int PulseSensorHRVal = analogRead(PulseSensorHRWire); // khởi tạo biến PulseSensorHRVal --> lấy và lưu giá trị đọc của biến PulseSensorHRWire trên chân A0 của bo mạch

    if (PulseSensorHRVal > Threshold && ThresholdStat == true) {// nếu như giá trị biến khởi tạo ở trên > giá trị ngưỡng và biến ThresholdStat(lấy giá trị ngưỡng) ==true 
      cntHB++; // biến đếm giá trị nhịp tim đập cntHB tăng 1 giá trị
      ThresholdStat = false; // đưa biến lấy giá trị ngưỡng ThresholdStat trở về false
      digitalWrite(LED_D1,HIGH);//đẩy LED_D1, led khai báo ở chân D1 lên high để bật led
      delay(500); // ngắt xung trong 1/2 s.
    }

    if (PulseSensorHRVal < Threshold) { // nếu như giá trị biến PulseSensorHRVal nhỏ hơn giá trị ngưỡng được đặt ra.
      ThresholdStat = true; // biến lấy giá trị ngưỡng ThresholdStat vẫn = true
      digitalWrite(LED_D1,LOW);//LED_D1 = LOW, led khai báo ở chân D1 vẫn tắt
    }
  }
  //----------------------------------------

  //---------------------------------------- chương trình tính toán BMP
  unsigned long currentMillisHR = millis(); //khởi tạo biến currentMillisHR để lấy giá trị thời gian đọc của nhịp tim

  if (currentMillisHR - previousMillisHR >= intervalHR) {// nếu như giá trị thời đọc hiện tại - thời gian đọc lần cuối >= giá trị thời gian để đọc nhịp tim = 10s
    previousMillisHR = currentMillisHR;// giá trị đọc lần cuối(previousMillisHR) = thời gian đọc hiện tại(currentMillisHR) 

    BPMval =  cntHB* 6 ; //--> Lấy giá trị đợi để tính được BMP là 10s, nên giá trị BMP = số lần đếm được nhịp đập * 6.
  
    Serial.print("BPM : "); // in ra màn serial giá trị BMP
    Serial.println(BPMval);// in ra màn serial giá trị BMP
    
    cntHB = 0;// đưa mạch đếm số lần nhịp đập = 0;
  }
  //----------------------------------------
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------void setup()
void setup() {

  
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(500);
    
  pinMode(ON_Board_LED,OUTPUT); //-->  để ON_Board_LED là output
  digitalWrite(ON_Board_LED, HIGH); //--> tắt đèn led chân D4.

  pinMode(LED_D1,OUTPUT); //-->  để LED_D1 là output

  WiFi.mode(WIFI_STA);//chế độ WIFI là station mode
  WiFi.begin(ssid, password); //-->  esp8266 kết nối với Wifi
  Serial.println("");

  //----------------------------------------đợi connection, đợi wifi kết nối với esp8266
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { //nếu wifi chưa kết nối được
    Serial.print(".");// in ra "."
  
    digitalWrite(ON_Board_LED, LOW); 
    delay(250);
    digitalWrite(ON_Board_LED, HIGH);
    delay(250);
    //--> nhấp nháy led on board
    //----------------------------------------
  }
  //----------------------------------------
  digitalWrite(ON_Board_LED, HIGH); //--> tắt led khi bo đã kết nối được với wifi
  //---------------------------------------- nếu như kết nối với wifi thành công, địa chỉ ip sẽ được in ra ở serial monitor
  Serial.println("");
  Serial.print("Đã kết nối đến: ");
  Serial.println(ssid);
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  server.on("/", handleRoot); //--> Server.on là đặt chức năng gọi lại hàm, khi máy chủ liên kết với máy khách và khi có một SSE xảy ra (ServerSendEvents) thì trong 
  //trường hợp trên hàm handleRoot sẽ được gọi ra với phương thức GET, bất cứ khi nào có sự kiện liên quan đến, thì hàm trên sẽ tìm sự yêu cầu phù hợp với lại yêu cầu và tiếp đến là hàm liên quan
  server.on("/getHeartRate", handleHeartRate); 
  // gọi lại hàm handleHeartRate và liên kết qua webpage, sau khi có SeverSendEvent

  server.begin(); //--> Start server. khởi tạo server
  Serial.println("Đã khởi tạo được Webserver");

  Serial.println();
  Serial.println("Đợi 10 giây để có được giá trị BPM");
}
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------void loop()
void loop() {
 

  server.handleClient(); // hàm xử lý các SSE, sever.handCilent sẽ gọi các hàm được thiết lập trong server.on() miễn là có SSE được truyền đến
  GetHeartRate(); // gọi hàm GetHeartRate

}