float buffer[1000];
int my_index = 0;

int myIndex()
{
    return my_index;
}

float *getBuffer()
{
    return buffer;
}

void addValue(float val)
{
    buffer[my_index] = val;
    my_index++;
}

void initBuffer()
{
    my_index = 0;
    for (int i = 0; i < 1000; i++)
    {
        buffer[i] = -100000.69;
    }
}

void displayBuffer()
{
    for (int i = 0; i < my_index; i++)
    {
        Serial.println(buffer[i]);
        delay(500);
    }
}