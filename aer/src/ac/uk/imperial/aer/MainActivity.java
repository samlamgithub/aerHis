package ac.uk.imperial.aer;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.util.Log;

public class MainActivity extends Activity {
  private static final String TAG = "Aer";

  private FrameLayout mFrameLayout;

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
