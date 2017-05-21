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

  private TextView infoView;
  private static final int infoViewID = 9003;
  String text = "";
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
    writingSwitcher.setTextOn("Tap to Stop writing");
    writingSwitcher.setBackgroundColor(Color.WHITE);
    writingSwitcher.setTextColor(Color.BLACK);
    writingSwitcher.setId(writingSwitcherID);
    writingSwitcher.setTextOff("Tap to Start writing");
    writingSwitcher.setChecked(false);
    //    writingSwitcher.setWidth(250);
    writingSwitcher.setOnClickListener(this);

    elasticFusionSwitcher = new ToggleButton(this);
    elasticFusionSwitcher.setTextOn("Tap to Stop ElasticFusion");
    elasticFusionSwitcher.setBackgroundColor(Color.WHITE);
    elasticFusionSwitcher.setTextColor(Color.BLACK);
    elasticFusionSwitcher.setId(elasticFusionSwitcherID);
    elasticFusionSwitcher.setTextOff("Tap to Start ElasticFusion");
    elasticFusionSwitcher.setChecked(false);
    //    elasticFusionSwitcher.setWidth(350);
    elasticFusionSwitcher.setOnClickListener(this);

    infoView = new TextView(this);
    // infoView.setTextSize(17);
    // infoView.setPadding(5, 3, 0, 3);
    // infoView.setTypeface(Typeface.DEFAULT_BOLD);
    infoView.setGravity(Gravity.START | Gravity.CENTER);
    infoView.setText("info View");
    infoView.setId(infoViewID);
    infoView.setLayoutParams(
        new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT));

    savePlybutton = new Button(this);
    savePlybutton.setText("Save ply");
    savePlybutton.setBackgroundColor(Color.WHITE);
    savePlybutton.setTextColor(Color.BLACK);
    savePlybutton.setId(savePlybuttonID);
    //    savePlybutton.setWidth(250);
    savePlybutton.setOnClickListener(this);

    LinearLayout ll = new LinearLayout(this);
    ll.setBackgroundColor(Color.WHITE);
    ll.setOrientation(LinearLayout.HORIZONTAL);
    ll.addView(writingSwitcher);
    ll.addView(elasticFusionSwitcher);
    ll.addView(savePlybutton);
    ll.addView(infoView);

    LinearLayout.LayoutParams layoutParams =
        new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT,
                                      ViewGroup.LayoutParams.WRAP_CONTENT);

    addContentView(ll, layoutParams);

    infoThread = new Thread() {
      public void run() {
        while (true) {
          MemoryInfo mi = new MemoryInfo();
          ActivityManager activityManager =
              (ActivityManager)getSystemService(ACTIVITY_SERVICE);
          activityManager.getMemoryInfo(mi);
          double availableMegs = mi.availMem / 0x100000L;
          // Percentage can be calculated for API 16+
          double percentAvail = mi.availMem / (double)mi.totalMem;

          final Runtime runtime = Runtime.getRuntime();
          long freeSize = runtime.freeMemory();
          long totalSize = runtime.totalMemory();
          final long usedMemInMB = (totalSize - freeSize) / 1048576L;
          final long maxHeapSizeInMB = runtime.maxMemory() / 1048576L;
          final long availHeapSizeInMB = maxHeapSizeInMB - usedMemInMB;

          float cpuUsage = 0;

          try {
            RandomAccessFile reader = new RandomAccessFile("/proc/stat", "r");
            String load = reader.readLine();

            String[] toks = load.split(" +"); // Split on one or
                                              // more spaces
            long idle1 = Long.parseLong(toks[4]);
            long cpu1 = Long.parseLong(toks[2]) + Long.parseLong(toks[3]) +
                        Long.parseLong(toks[5]) + Long.parseLong(toks[6]) +
                        Long.parseLong(toks[7]) + Long.parseLong(toks[8]);

            try {
              Thread.sleep(360);
            } catch (Exception e) {
            }

            reader.seek(0);
            load = reader.readLine();
            reader.close();

            toks = load.split(" +");

            long idle2 = Long.parseLong(toks[4]);
            long cpu2 = Long.parseLong(toks[2]) + Long.parseLong(toks[3]) +
                        Long.parseLong(toks[5]) + Long.parseLong(toks[6]) +
                        Long.parseLong(toks[7]) + Long.parseLong(toks[8]);

            cpuUsage = (float)(cpu2 - cpu1) / ((cpu2 + idle2) - (cpu1 + idle1));

          } catch (IOException ex) {
            ex.printStackTrace();
          }

          String availableMegsStr = String.valueOf(availableMegs);
          String percentAvailStr = String.valueOf(percentAvail);
          String availHeapSizeInMBStr = String.valueOf(availHeapSizeInMB);
          String cpuUsageStr = String.valueOf(cpuUsage);
          text = availableMegsStr + " " + percentAvailStr + " " +
                 availHeapSizeInMBStr + " " + cpuUsageStr;
          Log.v("info", text);

          runOnUiThread(new Runnable() {

            public void run() { infoView.setText(text); }
          });
        }
      }
    };

    infoThread.start();

    //    addContentView(writingSwitcher, new
    //    LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
    //    addContentView(elasticFusionSwitcher, new
    //    LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
    //    addContentView(savePlybutton, new
    //    LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
    //    writingSwitcher = (ToggleButton) findViewById(R.id.log_writer);
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
      // JNIInterface.imuOnPause();
    }
  }

  /**
   * The writing button was pressed.
   */
  public void onClick(View view) {
    switch (view.getId()) {
    case writingSwitcherID:
      JNIInterface.setWriting(writingSwitcher.isChecked());
      break;
    case elasticFusionSwitcherID:
      JNIInterface.setElasticFusion(elasticFusionSwitcher.isChecked());
      break;
    case savePlybuttonID:
      JNIInterface.savePly();
      break;
    default:
      break;
    }
  }

  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();
    // JNIInterface.imuOnResume();
  }

  public void surfaceCreated() {
    mIsCameraConnected = JNIInterface.cameraConnect();
    if (!mIsCameraConnected) {
      Log.e(TAG, "Failed to connect to camera");
      finish();
    }
  }

  public void requestRender() { mGLView.requestRender(); }
}
