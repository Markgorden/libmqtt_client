#include "mqtt_client/MQTTClient.h"


typedef struct PACKED         //定义一个cpu occupy的结构体
{
    char name[20];      //定义一个char类型的数组名name有20个元素
    unsigned int user; //定义一个无符号的int类型的user
    unsigned int nice; //定义一个无符号的int类型的nice
    unsigned int system;//定义一个无符号的int类型的system
    unsigned int idle; //定义一个无符号的int类型的idle
}CPU_OCCUPY;

typedef struct __tagMqttClientContext
{
    MQTTClient client;
    
    CPU_OCCUPY cpu_stat1;
    CPU_OCCUPY cpu_stat2;
    
} MQTT_CLIENT_CONTEXT;

static MQTT_CLIENT_CONTEXT mqtt_client_context = {0};
/*
static int cmd_opr(char* payload)
{
    switch(policy->functioncode)
    {
        case MODBUS_FC_READ_COILS:

            break;

        case MODBUS_FC_READ_DISCRETE_INPUTS:
            break;
    
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            break;

        case MODBUS_FC_READ_INPUT_REGISTERS:
            break;

        default:
            fprintf(stderr, "not supported function code:%d\n", policy->functioncode);
            break;
    }
}
*/


static void get_cpuoccupy (CPU_OCCUPY *cpust) //对无类型get函数含有一个形参结构体类弄的指针O
{   
    FILE *fd;         
    int n;            
    char buff[256]; 
    CPU_OCCUPY * cpu_occupy;
    cpu_occupy = cpust;
    fd = fopen ("/proc/stat", "r"); 
    fgets(buff, sizeof(buff), fd);
    sscanf(buff, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle);
    fclose(fd);
}

int cal_cpuoccupy (CPU_OCCUPY *o, CPU_OCCUPY *n) 
{   
    unsigned long od, nd;    
    unsigned long id, sd;
    int cpu_use = 0;   

    od = (unsigned long) (o->user + o->nice + o->system +o->idle);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (unsigned long) (n->user + n->nice + n->system +n->idle);//第二次(用户+优先级+系统+空闲)的时间再赋给od
      
    id = (unsigned long) (n->user - o->user);    //用户第一次和第二次的时间之差再赋给id
    sd = (unsigned long) (n->system - o->system);//系统第一次和第二次的时间之差再赋给sd
    if((nd-od) != 0)
    cpu_use = (int)((sd+id)*10000)/(nd-od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else cpu_use = 0;
    //printf("cpu: %u/n",cpu_use);
    return cpu_use;
}

#define ADDRESS     "tcp://120.25.158.163:1883"
#define CLIENTID    "Client_1"
#define TOPIC       "cpu"
//#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

int pub_message_ex(char * PAYLOAD)
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        //exit(-1);
    }
    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    printf("Waiting for up to %d seconds for publication of %s\n"
            "on topic %s for client with ClientID: %s\n",
            (int)(TIMEOUT/1000), PAYLOAD, TOPIC, CLIENTID);
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

bool main()
{
    MQTT_CLIENT_CONTEXT * c = &mqtt_client_context;
    // get cpu status.
    //第一次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&c->cpu_stat1);
    sleep(5);
    //第二次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&c->cpu_stat2);
    //计算cpu使用率
    int cpu = cal_cpuoccupy ((CPU_OCCUPY *)&c->cpu_stat1, (CPU_OCCUPY *)&c->cpu_stat2);
    // send message to cloud.
    char message[100];
    sprintf(message,"当前cpu占用率%d",cpu);
    printf("%s\n",message);
    //pub_message(message);
    pub_message_ex(message);
}

