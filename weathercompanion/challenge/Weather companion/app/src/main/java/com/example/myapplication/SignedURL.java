package com.example.myapplication;
import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;
import android.widget.TextView;
import com.google.auth.oauth2.ServiceAccountCredentials;
import com.google.cloud.storage.Storage;
import com.google.cloud.storage.StorageOptions;
import com.google.cloud.storage.BlobInfo;
import com.google.cloud.storage.Storage.SignUrlOption;
import org.json.JSONException;
import org.json.JSONObject;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.concurrent.TimeUnit;

public class SignedURL extends AsyncTask<Void, Void, String> {

    private final MainActivity mainActivity;
    private final Context appContext;
    public SignedURL(MainActivity mainActivity,Context appContext) throws IOException {
        this.mainActivity = mainActivity;
        this.appContext = appContext;
    }
    protected String doInBackground(Void... blah) {
        String url = null;
        String response = "";
        Storage storage = StorageOptions.getDefaultInstance().getService();
        String bucketName = "weather-companion";
        String blobName = "weather.json";
        try {
            Utils utils = new Utils(appContext);
            String keyJson = utils.getKey();
            URL signedUrl = storage.signUrl(BlobInfo.newBuilder(bucketName, blobName).build(),
            14, TimeUnit.DAYS, SignUrlOption.signWith(ServiceAccountCredentials.fromStream(new ByteArrayInputStream(keyJson.getBytes()))));
            url = signedUrl.toString();
            URLConnection connection = (HttpURLConnection)signedUrl.openConnection();
            connection.connect();
            int contentLength =  connection.getContentLength();
            BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            StringBuffer buffer = new StringBuffer();
            while ((response = reader.readLine()) != null) {
                buffer.append(response+"\n");

            }
            response = buffer.toString();
        } catch (IOException e) {
            e.printStackTrace();
        }
        if(url != null) {
            Log.d("Background url", "Signed URL created");
        }
        return response;
    }
    protected void onPostExecute(String response) {
        Log.d("Complete reponse",response);
        try {
            JSONObject obj = new JSONObject(response);
            JSONObject location = (JSONObject)obj.get("display_location");
            String city = location.getString("city");
            JSONObject weather = (JSONObject) obj.get("current_weather");
            String temperature = weather.getString("temperature");
            String precipitation = weather.getString("precipitation");
            String humidity = weather.getString("humidity");
            String wind = weather.getString("wind");
            TextView mTextView = (TextView) mainActivity.findViewById(R.id.cityName);
            mTextView.setText(city);
            mTextView = (TextView) mainActivity.findViewById(R.id.precipitationValue);
            mTextView.setText(precipitation);
            mTextView = (TextView) mainActivity.findViewById(R.id.humidityValue);
            mTextView.setText(humidity);
            mTextView = (TextView) mainActivity.findViewById(R.id.windValue);
            mTextView.setText(wind);
            mTextView = (TextView) mainActivity.findViewById(R.id.temperatureValue);
            mTextView.setText(temperature);


        } catch (JSONException e) {
            e.printStackTrace();
        }

    }
    }

