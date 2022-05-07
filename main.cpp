#include "mbed.h"
#include <Ticker.h>
#include "drivers/DigitalOut.h"

#include "erpc_simple_server.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "UARTTransport.h"
#include "DynamicMessageBufferFactory.h"
#include "hw3_server.h"

#include "stm32l475e_iot01_accelero.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
/*
 * Macros for setting console flow control.
 */
#define CONSOLE_FLOWCONTROL_RTS 1
#define CONSOLE_FLOWCONTROL_CTS 2
#define CONSOLE_FLOWCONTROL_RTSCTS 3
#define mbed_console_concat_(x) CONSOLE_FLOWCONTROL_##x
#define mbed_console_concat(x) mbed_console_concat_(x)
#define CONSOLE_FLOWCONTROL mbed_console_concat(MBED_CONF_TARGET_CONSOLE_UART_FLOW_CONTROL)

Thread AccelThread;
Ticker AccelTicker;
EventQueue AccelAndPublishQueue;
WiFiInterface *wifi;
MQTT::Client<MQTTNetwork, Countdown>* client;
volatile int message_num = 0;
volatile int arrivedcount = 0;
volatile bool closed = false;
const char* topic = "Mbed";
int16_t pDataXYZ[3] = {0};
bool StartFlag = 0;

void publish_message() {
    message_num++;
    MQTT::Message message;
    char buff[150];
    if (StartFlag)
        sprintf(buff, "Accel: X : %d, Y : %d, Z : %d", pDataXYZ[0], pDataXYZ[1], pDataXYZ[2]);
    else
        sprintf(buff, "End");;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*) buff;
    message.payloadlen = strlen(buff) + 1;
    int rc = client->publish(topic, message);

    if (message_num % 10 == 0)
    {
        printf("rc: %d\r\n", rc);
        printf("#%d Puslish message: %s\r\n", message_num, buff);
    }
    
    //printf("Accel: X : %d, Y : %d, Z : %d\r\n", pDataXYZ[0], pDataXYZ[1], pDataXYZ[2]);
}
void getAccelAndPublish(void)
{
    BSP_ACCELERO_AccGetXYZ(pDataXYZ);
    publish_message();
}
/****** erpc declarations *******/
void service_on(void)
{
    StartFlag = 1;
    AccelTicker.attach(AccelAndPublishQueue.event(getAccelAndPublish), 100ms);
}

void service_off(void)
{
    StartFlag = 0;
    AccelAndPublishQueue.call(publish_message);
    AccelAndPublishQueue.call([]{AccelTicker.detach();});
}

/** erpc infrastructure */
ep::UARTTransport uart_transport(D1, D0, 9600);
ep::DynamicMessageBufferFactory dynamic_mbf;
erpc::BasicCodecFactory basic_cf;
erpc::Crc16 crc16;
erpc::SimpleServer rpc_server;
/* service */
accelerometer_service accel_service;


int main()
{
    uart_transport.setCrc16(&crc16);
    BSP_ACCELERO_Init();
    AccelThread.start(callback(&AccelAndPublishQueue, &EventQueue::dispatch_forever));
  // Set up hardware flow control, if needed
#if CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTS
    uart_transport.set_flow_control(mbed::SerialBase::RTS, STDIO_UART_RTS, NC);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_CTS
    uart_transport.set_flow_control(mbed::SerialBase::CTS, NC, STDIO_UART_CTS);
#elif CONSOLE_FLOWCONTROL == CONSOLE_FLOWCONTROL_RTSCTS
    uart_transport.set_flow_control(mbed::SerialBase::RTSCTS, STDIO_UART_RTS, STDIO_UART_CTS);
#endif

    wifi = WiFiInterface::get_default_instance();
    if (!wifi) {
            printf("ERROR: No WiFiInterface found.\r\n");
            return -1;
    }
    printf("\nConnecting to %s...\r\n", MBED_CONF_APP_WIFI_SSID);
    int ret = wifi->connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
            printf("\nConnection error: %d\r\n", ret);
            return -1;
    }
    NetworkInterface* net = wifi;
    MQTTNetwork mqttNetwork(net);
    client = new MQTT::Client<MQTTNetwork, Countdown>(mqttNetwork);
    //TODO: revise host to your IP
    const char* host = "192.168.20.33";
    const int port=1883;
    printf("Connecting to TCP network...\r\n");
    printf("address is %s/%d\r\n", host, port);
    int rc = mqttNetwork.connect(host, port);//(host, 1883);
    if (rc != 0) {
            printf("Connection error.");
            return -1;
    }
    printf("Successfully connected!\r\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "Mbed";
    if ((rc = client->connect(data)) != 0){
            printf("Fail to connect MQTT\r\n");
    }


    printf("Initializing server.\n");
    rpc_server.setTransport(&uart_transport);
    rpc_server.setCodecFactory(&basic_cf);
    rpc_server.setMessageBufferFactory(&dynamic_mbf);

    // Add the led service to the server
    rpc_server.addService(&accel_service);
    printf("ADDing server.\n");
    

    // Run the server. This should never exit
    printf("Running server.\n");
    rpc_server.run();
    

    //service_on();
    /*
    while (1)
    {
        getAccelAndPublish();
        client->yield(500);
        ThisThread::sleep_for(500ms);
    }
    */
}