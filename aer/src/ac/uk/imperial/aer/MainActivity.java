package ac.uk.imperial.aer;

import android.app.Activity;
import android.graphics.Color;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import java.io.IOException;
import java.io.RandomAccessFile;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.Color;
import android.graphics.Typeface;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.Button;
import android.util.Log;
import android.widget.ToggleButton;
import android.widget.Button;
import android.util.Log;
import java.io.IOException;
import java.io.RandomAccessFile;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.MemoryInfo;
import android.graphics.Color;
import android.graphics.Typeface;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.Button;
import android.util.Log;

public class MainActivity extends Activity implements OnClickListener {
  private static final String TAG = "Aer";

  private FrameLayout mFrameLayout;

  private ToggleButton writingSwitcher;
  private static final int writingSwitcherID = 9000;

  private ToggleButton elasticFusionSwitcher;
  private static final int elasticFusionSwitcherID = 9001;

  private Button savePlybutton;
  private static final int savePlybuttonID = 9002;

  private ToggleButton runDataSetButton;
  private static final int runDataSetbuttonID = 9003;

  private TextView infoView;
  private static final int infoViewID = 9004;

  private int counter = 0;
  private TextView counterView;
  private static final int counterViewID = 9005;

  private ToggleButton tangoRGBDDataButton;
  private static final int tangoRGBDDataButtonID = 9006;

  private ToggleButton tangoRGBDPoseDataButton;
  private static final int tangoRGBDPoseDataButtonID = 9007;

  private String text = "";
  Thread infoThread;

  private boolean mIsCameraConnected = false;
  private GLSurfaceRenderer mRenderer;
  private GLSurfaceView mGLView;

  private CameraOverlay mOverlay;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                         WindowManager.LayoutParams.FLAG_FULLSCREEN);
    JNIInterface.cameraInitialise(this, getAssets());
    mFrameLayout = new FrameLayout(this);
    mOverlay = new CameraOverlay(this);
    addContentView(mOverlay, new LayoutParams(LayoutParams.MATCH_PARENT,
                                              LayoutParams.MATCH_PARENT));
    mGLView = new GLSurfaceView(this);
    mRenderer = new GLSurfaceRenderer(this, mOverlay);
    mGLView.setRenderer(mRenderer);
    mGLView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    addContentView(mGLView, new LayoutParams(LayoutParams.MATCH_PARENT,
                                             LayoutParams.MATCH_PARENT));
    // add Toggle button
    writingSwitcher = new ToggleButton(this);
    writingSwitcher.setTextOn("Stop writing");
    writingSwitcher.setBackgroundColor(Color.WHITE);
    writingSwitcher.setTextColor(Color.BLACK);
    writingSwitcher.setId(writingSwitcherID);
    writingSwitcher.setTextOff("Start writing");
    writingSwitcher.setChecked(false);
    writingSwitcher.setOnClickListener(this);

    elasticFusionSwitcher = new ToggleButton(this);
    elasticFusionSwitcher.setTextOn("Stop ElasticFusion");
    elasticFusionSwitcher.setBackgroundColor(Color.WHITE);
    elasticFusionSwitcher.setTextColor(Color.BLACK);
    elasticFusionSwitcher.setId(elasticFusionSwitcherID);
    elasticFusionSwitcher.setTextOff("Start ElasticFusion");
    elasticFusionSwitcher.setChecked(false);
    elasticFusionSwitcher.setOnClickListener(this);

    infoView = new TextView(this);
    infoView.setGravity(Gravity.START | Gravity.CENTER);
    infoView.setText("info View");
    infoView.setId(infoViewID);
    infoView.setLayoutParams(
        new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));

    counterView = new TextView(this);
    counterView.setGravity(Gravity.START | Gravity.CENTER);
    counterView.setText("frame count: 0 ");
    counterView.setId(counterViewID);
    counterView.setLayoutParams(
        new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));

    savePlybutton = new Button(this);
    savePlybutton.setText("Save ply");
    savePlybutton.setBackgroundColor(Color.WHITE);
    savePlybutton.setTextColor(Color.BLACK);
    savePlybutton.setId(savePlybuttonID);
    savePlybutton.setOnClickListener(this);

    runDataSetButton = new ToggleButton(this);
    runDataSetButton.setTextOn("stop Run dataset");
    runDataSetButton.setBackgroundColor(Color.WHITE);
    runDataSetButton.setTextColor(Color.BLACK);
    runDataSetButton.setId(runDataSetbuttonID);
    runDataSetButton.setTextOff("Run dataset");
    runDataSetButton.setChecked(false);
    runDataSetButton.setOnClickListener(this);

    tangoRGBDDataButton = new ToggleButton(this);
    tangoRGBDDataButton.setTextOn("stop tangoRGBDData");
    tangoRGBDDataButton.setBackgroundColor(Color.WHITE);
    tangoRGBDDataButton.setTextColor(Color.BLACK);
    tangoRGBDDataButton.setId(tangoRGBDDataButtonID);
    tangoRGBDDataButton.setTextOff("Run tangoRGBDData");
    tangoRGBDDataButton.setChecked(false);
    tangoRGBDDataButton.setOnClickListener(this);

    tangoRGBDPoseDataButton = new ToggleButton(this);
    tangoRGBDPoseDataButton.setTextOn("stop tangoRGBDPoseData");
    tangoRGBDPoseDataButton.setBackgroundColor(Color.WHITE);
    tangoRGBDPoseDataButton.setTextColor(Color.BLACK);
    tangoRGBDPoseDataButton.setId(tangoRGBDPoseDataButtonID);
    tangoRGBDPoseDataButton.setTextOff("Run tangoRGBDPoseData");
    tangoRGBDPoseDataButton.setChecked(false);
    tangoRGBDPoseDataButton.setOnClickListener(this);

    LinearLayout ll = new LinearLayout(this);
    ll.setBackgroundColor(Color.WHITE);
    ll.setOrientation(LinearLayout.HORIZONTAL);
    ll.addView(writingSwitcher);
    ll.addView(elasticFusionSwitcher);
    ll.addView(runDataSetButton);
    ll.addView(tangoRGBDDataButton);
    ll.addView(tangoRGBDPoseDataButton);
    ll.addView(savePlybutton);
    // ll.addView(infoView);
    ll.addView(counterView);

    LinearLayout.LayoutParams layoutParams =
        new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                                      ViewGroup.LayoutParams.WRAP_CONTENT);

    addContentView(ll, layoutParams);
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
  }

  @Override
  protected void onPause() {
    super.onPause();
    mGLView.onPause();
    if (infoThread != null) {
      try {
        infoThread.join();
      } catch (InterruptedException e) {
        // TODO Auto-generated catch block
        e.printStackTrace();
      }
    }
    if (mIsCameraConnected) {
      JNIInterface.cameraDisconnect();
    }
  }

  /**
   * The writing button was pressed.
   */
  public void onClick(View view) {
    switch (view.getId()) {
    case writingSwitcherID:
      boolean b = JNIInterface.setWriting(writingSwitcher.isChecked());
      if (!b) {
        String action = "start";
        if (!writingSwitcher.isChecked()) {
          action = "stop";
        }
        popError("Unable to " + action + " write log data.");
        writingSwitcher.setChecked(!writingSwitcher.isChecked());
      }
      break;
    case elasticFusionSwitcherID:
      boolean b2 =
          JNIInterface.setElasticFusion(elasticFusionSwitcher.isChecked());
      if (!b2) {
        String action = "start";
        if (!elasticFusionSwitcher.isChecked()) {
          action = "stop";
        }
        popError("Unable to " + action + " elasticfusion.");
        elasticFusionSwitcher.setChecked(!elasticFusionSwitcher.isChecked());
      }
      break;
    case runDataSetbuttonID:
      boolean b3 = JNIInterface.setRunDataSet(runDataSetButton.isChecked());
      if (!b3) {
        String action = "start";
        if (!runDataSetButton.isChecked()) {
          action = "stop";
        }
        popError("Unable to " + action + " run dataset.");
        runDataSetButton.setChecked(!runDataSetButton.isChecked());
      }
      break;
    case tangoRGBDDataButtonID:
      boolean b4 =
          JNIInterface.setRuntangoRGBDData(tangoRGBDDataButton.isChecked());
      if (!b4) {
        String action = "start";
        if (!tangoRGBDDataButton.isChecked()) {
          action = "stop";
        }
        popError("Unable to " + action + " run tangoRGBDData.");
        tangoRGBDDataButton.setChecked(!tangoRGBDDataButton.isChecked());
      }
      break;
    case tangoRGBDPoseDataButtonID:
      boolean b5 = JNIInterface.setRuntangoRGBDPoseData(
          tangoRGBDPoseDataButton.isChecked());
      if (!b5) {
        String action = "start";
        if (!tangoRGBDPoseDataButton.isChecked()) {
          action = "stop";
        }
        popError("Unable to " + action + " run tangoRGBDPoseData.");
        tangoRGBDPoseDataButton.setChecked(
            !tangoRGBDPoseDataButton.isChecked());
      }
      break;
    case savePlybuttonID:
      boolean b6 = JNIInterface.savePly();
      if (!b6) {
        popError("Unable to savePly");
      }
      break;
    default:
      break;
    }
  }

  public void popError(String error) {
    AlertDialog.Builder builder = new AlertDialog.Builder(this);
    builder.setMessage(error).setPositiveButton(
        "OK", new DialogInterface.OnClickListener() {
          public void onClick(DialogInterface dialog, int id) {
            dialog.dismiss();
          }
        });

    builder.show();
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();
  }

  public void surfaceCreated() {
    mIsCameraConnected = JNIInterface.cameraConnect();
    if (!mIsCameraConnected) {
      Log.e(TAG, "Failed to connect to camera");
      finish();
    }
  }

  public void resetCounter() {
    Log.v("elasticfusion info ", "resetCounter in java called");
    new Thread() {
      public void run() {
        runOnUiThread(new Runnable() {
          public void run() { counterView.setText("current frame count: 0"); }
        });
      }
    }.start();
  }

  public void incrementCounter() {
    Log.v("elasticfusion info ", "incrementCounter in java called");
    new Thread() {
      public void run() {
        runOnUiThread(new Runnable() {
          public void run() {
            counter++;
            counterView.setText(
                "current frame count: " + Integer.toString(counter) + " ");
          }
        });
      }
    }.start();
  }

  public void requestRender() { mGLView.requestRender(); }
}
