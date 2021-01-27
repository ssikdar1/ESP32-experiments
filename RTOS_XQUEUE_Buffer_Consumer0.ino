#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// A simple Producer-Consumer using freeRTOS xQueue

QueueHandle_t xQueue;

// Fun to have the led blink to know things are okay
void TaskBlink(void *pvParameters)
{
  (void) pvParameters;

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(100);
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(100);
  }
}

void TaskProducer(void *pvParameters)
{
  (void) pvParameters;

  Serial.println("TaskProducer");

  int foo = 0;   // int to go onto the queue
  
  for (;;)
  {
   Serial.println("TaskProducer");
   if( xQueue != NULL )
   {
      Serial.print("Producer Task Queue freespace: "); Serial.println(uxQueueSpacesAvailable(xQueue));
      if (uxQueueSpacesAvailable == 0)
      {
        Serial.println("Giving Consumer some breathing room!"); // Not reaching this stmt?
        vTaskDelay(3000); // Be nice and let the Consumer try and catchup
      }
      if ( xQueueSend( 
             xQueue,
            ( void * ) &foo,
            ( TickType_t ) 10 ) != pdPASS ) 
     {
        Serial.println("Queue is full");
        Serial.print("Will try to put "); Serial.print(foo); Serial.println(" next time.");
        vTaskDelay(10000);
     } else {
        Serial.print(foo); Serial.println(" was put onto the queue");
        foo++;
     }
   }
   vTaskDelay(2000);
  } 
}

void TaskConsumer(void *pvParameters)
{
  for (;;)
  {
    if( xQueue != NULL )
    {
      Serial.print("Consumer Task Queue freespace: "); Serial.println(uxQueueSpacesAvailable(xQueue));
      int bar;
      if( xQueueReceive( xQueue,
                           &( bar ),
                           ( TickType_t ) 10 ) == pdPASS )
      {
          Serial.print(bar); Serial.println(" was Consumed off the queue.");
      }
     } else {
      Serial.println("xQueue was null");  
    }
    vTaskDelay(4000);
  }
}

void setup() 
{
  Serial.begin(115200);
  xQueue = xQueueCreate( 5, sizeof( int ) );

  xTaskCreatePinnedToCore(
  TaskBlink                 // Pointer to the task entry function
  ,  "TaskBlink"            // descriptive name for the task
  ,  1024                   // The size of the task stack specified as the number of bytes. Can be checked & adjusted by reading the Stack Highwater
  ,  NULL                   // Pointer to pvParameters for the task being created
  ,  2                      // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
  ,  NULL                   // Used to pass back a handle by which the created task can be referenced.
  ,  ARDUINO_RUNNING_CORE   // If the value is tskNO_AFFINITY, the created task is not pinned to any CPU, and the scheduler can run it on any core available.
                            // Values 0 or 1 indicate the index number of the CPU which the task should be pinned to.
  );

  xTaskCreatePinnedToCore(
    TaskProducer
    ,  "TaskProducer"
    ,  20 * 1024 // Stack Highwater: 54bit -> 2*1025 is minimum
    ,  NULL
    ,  2
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

   xTaskCreatePinnedToCore(
    TaskConsumer
    ,  "TaskConsumer"
    ,  10 * 1024 // Stack Highwater: 54bit -> 2*1025 is minimum
    ,  NULL
    ,  3
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);   
}

void loop() 
{
  // Not used since everything runs in tasks
}
