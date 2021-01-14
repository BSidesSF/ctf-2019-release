package com.example.yayornay;

import java.util.Date;

public class Location {
    Date date;
    double latitude, longitude;
    float color;
    Location(Date date, double latitude, double longitude, float color){
        this.date = date;
        this.latitude = latitude;
        this.longitude = longitude;
        this.color = color;
    }
}
