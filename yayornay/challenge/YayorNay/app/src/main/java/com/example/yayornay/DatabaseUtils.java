package com.example.yayornay;


import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteStatement;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.Log;
import java.lang.Math;
import android.widget.Toast;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;


public class DatabaseUtils extends SQLiteOpenHelper {
    private static String pathToDB = "/data/data/com.example.yayornay/databases/";
    private static String dbName = "Location.db";
    private SQLiteDatabase db;
    private final Context appcontext;

    public DatabaseUtils(Context context) throws IOException {

        super(context, dbName, null, 1);
        this.appcontext = context;
        this.createDB();
    }
    public void createDB() throws IOException {
        SQLiteDatabase test = null;
        try {
            test = SQLiteDatabase.openDatabase(pathToDB + dbName, null, SQLiteDatabase.OPEN_READONLY);
            Log.d("Exists","yes");
        }
        catch (SQLiteException e){}
        if (test == null){
            SQLiteDatabase database = this.getReadableDatabase();
            database.close();
            InputStream input = appcontext.getAssets().open(dbName);
            OutputStream output = new FileOutputStream(pathToDB + dbName);
            byte[] buffer = new byte[1024];
            int length;
            while((length = input.read(buffer))>0){
                output.write(buffer,0,length);
            }
            output.flush();
            output.close();
            input.close();
        }
    }
    public void openDB(){
        db = SQLiteDatabase.openDatabase(pathToDB + dbName, null, SQLiteDatabase.OPEN_READWRITE);
    }
    public ArrayList<Location> fetchLocations() throws IOException{
        this.openDB();
        // Test
        String test = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name";
        Cursor cursortest = db.rawQuery(test,null);
        while(cursortest.moveToNext()){
            Log.d("Table Name:",cursortest.getString(0));
        }
        cursortest.close();
        //End Test

        ArrayList<Location> locations = new ArrayList<Location>();
        String query = "SELECT * FROM locations";
        Cursor cursor = db.rawQuery(query, null);
        try {
            while (cursor.moveToNext()) {
                Location location = new Location(stringtoDate(cursor.getString(0)),cursor.getDouble(1), cursor.getDouble(2), cursor.getFloat(3));
                locations.add(location);
            }
        } finally {
            cursor.close();
        }
        return locations;
    }
    public String datetoString(Date date){
        String output;
        DateFormat df = new SimpleDateFormat("MM/dd/yyyy");
        output = df.format(date);
        return output;
    }
    public Date stringtoDate(String date){
        Date output = null;
        DateFormat df = new SimpleDateFormat("MM/dd/yyyy");
        try {
            output = df.parse(date);
        } catch (ParseException e) {
            e.printStackTrace();
        }
        return output;
    }
    @RequiresApi(api = Build.VERSION_CODES.N)
    public int numOfLocations() throws IOException{
        int count = 0;
        String query = "SELECT COUNT(*) FROM locations";
        SQLiteStatement stmt = db.compileStatement(query);
        count = Math.toIntExact(stmt.simpleQueryForLong());
        return count;
    }
    public void insertLocation(Location location) throws IOException {
        this.openDB();
        String query = "INSERT INTO locations (date, latitude, longitude, color) VALUES (?,?,?,?)";
        SQLiteStatement stmt = db.compileStatement(query);
        stmt.bindString(1,datetoString(location.date));
        stmt.bindDouble(2,location.latitude);
        stmt.bindDouble(3,location.longitude);
        stmt.bindDouble(4,location.color);
        stmt.executeInsert();
    }

    @Override
    public synchronized void close(){
        if(db != null)
        {
            db.close();
        }
        super.close();
    }
    @Override
    public void onCreate(SQLiteDatabase db) {

    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

    }
}
