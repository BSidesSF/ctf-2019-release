package com.example.yayornay;

import android.support.v4.app.FragmentActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.Toast;

import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.GoogleMap.OnMapLongClickListener;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.BitmapDescriptorFactory;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.MarkerOptions;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Date;

public class MapsActivity extends FragmentActivity implements OnMapReadyCallback, GoogleMap.OnMapClickListener, OnMapLongClickListener {

    private GoogleMap mMap;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_maps);
        // Obtain the SupportMapFragment and get notified when the map is ready to be used.
        SupportMapFragment mapFragment = (SupportMapFragment) getSupportFragmentManager()
                .findFragmentById(R.id.map);
        mapFragment.getMapAsync(this);

    }


    @Override
    public void onMapReady(GoogleMap googleMap) {
        mMap = googleMap;
        mMap.setOnMapLongClickListener(this);
        mMap.setOnMapClickListener(this);
        // Start at BsidesSF
        try {
            DatabaseUtils dbUtil = new DatabaseUtils(getApplicationContext());
            ArrayList<Location> locations = dbUtil.fetchLocations();
            for(Location location : locations) {
                LatLng temp = new LatLng(location.latitude, location.longitude);
                float color = BitmapDescriptorFactory.HUE_GREEN;
                String label = "Yay!";
                if(location.color == 0.0){
                    color = BitmapDescriptorFactory.HUE_RED;
                    label = "Nay!";
                }
                mMap.addMarker(new MarkerOptions().position(temp).title(label).icon(BitmapDescriptorFactory.defaultMarker(color)));
            }
            LatLng bSidesSF = new LatLng(37.7842927,-122.4037178);
            mMap.moveCamera(CameraUpdateFactory.newLatLng(bSidesSF));
            mMap.animateCamera( CameraUpdateFactory.zoomTo( 10.0f ) );
        }
        catch(IOException e){
            e.printStackTrace();
        }
    }

    public void onMapLongClick(LatLng point) {
        if (mMap != null) {
            mMap.addMarker(new MarkerOptions()
                    .position(point)
                    .title("Nay!")
                    .icon(BitmapDescriptorFactory.defaultMarker(BitmapDescriptorFactory.HUE_RED)));
        }
        try {
            DatabaseUtils dbUtil = new DatabaseUtils(getApplicationContext());
            Location location = new Location(new Date(System.currentTimeMillis()),point.latitude,point.longitude,BitmapDescriptorFactory.HUE_RED);
            dbUtil.insertLocation(location);
        }
        catch(IOException e){
            e.printStackTrace();
        }
    }

    public void onMapClick(LatLng point) {
        if (mMap != null) {
            mMap.addMarker(new MarkerOptions()
                    .position(point)
                    .title("Yay!")
                    .icon(BitmapDescriptorFactory.defaultMarker(BitmapDescriptorFactory.HUE_GREEN)));
        }
        try {
            DatabaseUtils dbUtil = new DatabaseUtils(getApplicationContext());
            Location location = new Location(new Date(System.currentTimeMillis()),point.latitude,point.longitude,BitmapDescriptorFactory.HUE_GREEN);
            dbUtil.insertLocation(location);
        }
        catch(IOException e){
            e.printStackTrace();
        }
    }
}
