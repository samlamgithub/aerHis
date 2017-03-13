package ac.uk.imperial.aer;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.RelativeLayout;
import android.widget.ToggleButton;
import android.util.Log;

public class MainActivity extends Activity implements OnClickListener{
  private static final String TAG = "Aer";

  private FrameLayout mFrameLayout;
  private ToggleButton writingSwitcher;
  private boolean mIsCameraConnected = false;
  private GLSurfaceRenderer mRenderer;
  private GLSurfaceView mGLView;

  private CameraOverlay mOverlay;
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
        WindowManager.LayoutParams.FLAG_FULLSCREEN);
    JNIInterface.cameraInitialise(this,getAssets());
    mFrameLayout = new FrameLayout(this); 
    mOverlay = new CameraOverlay(this);
    addContentView(mOverlay, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
    mGLView = new GLSurfaceView(this);
    mRenderer = new GLSurfaceRenderer(this,mOverlay);
    mGLView.setRenderer(mRenderer);
    mGLView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    addContentView(mGLView, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
    // add Toggle button
    writingSwitcher = new ToggleButton(this);
    writingSwitcher.setTextOn("Start writing");
    writingSwitcher.setId(9000);
    writingSwitcher.setTextOff("Stop writing");
    writingSwitcher.setChecked(false);
    writingSwitcher.setWidth(150);
    RelativeLayout.LayoutParams params = new RelativeLayout.LayoutParams(
    		RelativeLayout.ALIGN_PARENT_LEFT,
    		RelativeLayout.ALIGN_PARENT_TOP);
    writingSwitcher.setLayoutParams(params);
//    writingSwitcher.setLayoutParams(new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
    writingSwitcher.setOnClickListener(this);
//    <ToggleButton
//    android:id="@+id/log_writer"
//    android:layout_width="150dp"
//    android:layout_height="wrap_content"
//    android:text="Start writing"
//    android:onClick="writingClicked" />
    addContentView(writingSwitcher, new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));
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
    if (mIsCameraConnected) {
      JNIInterface.cameraDisconnect();
      JNIInterface.imuOnPause();
    }
  }
  
  /**
   * The writing button was pressed.
   */
  public void onClick(View view) {
	  switch (view.getId()) {
	  case 9000:
		  JNIInterface.setWriting(writingSwitcher.isChecked());
		  break;
	  }
  }


  @Override
  protected void onResume() {
    super.onResume();
    mGLView.onResume();
    JNIInterface.imuOnResume();
  }
  
  public void surfaceCreated() {
    mIsCameraConnected = JNIInterface.cameraConnect();
    if (!mIsCameraConnected) {
        Log.e(TAG, "Failed to connect to camera");
        finish();
    }
  }
  
  public void requestRender() {
    mGLView.requestRender();
  }
  
}
