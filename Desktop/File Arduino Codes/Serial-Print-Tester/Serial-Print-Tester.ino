int num = 0;

void setup() 
{
  Serial.begin(9600);
}


void loop() 
{
  function();
}


void function()
{
  Serial.println(num);
  num +=1;
  delay(300);
}