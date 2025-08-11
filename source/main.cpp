#include <iostream>

#include "sql_commands.h"
#include "datatypes.h"



int main() {
    std::cout << "Hello World!";
    SQLDatabase db("data.db");

    DataEntry data(RocketSensorData(5234, 12, 123));
    DataEntry data2(RocketKinematicsData(0.1, 0.2, 0.3, 12, 13));

    db.insert_data(data, 1);
    db.insert_data(data2, 1);
}