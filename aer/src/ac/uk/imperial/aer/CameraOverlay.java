package ac.uk.imperial.aer;

import java.util.ArrayList;

import android.content.Context;
import android.graphics.Canvas;
import android.util.Log;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff.Mode;

class CameraOverlay extends SurfaceView implements SurfaceHolder.Callback {
  private SurfaceHolder mHolder;
  private Paint mPaint;
  private int mCameraFrameHeight;
  private int mCameraFrameWidth;

  public CameraOverlay(MainActivity mainActivity) {
      super(mainActivity);
      setZOrderOnTop(true);
      mHolder = getHolder();
      mHolder.addCallback(this);
      mHolder.setFormat(PixelFormat.TRANSPARENT);
      mPaint = new Paint();
      mPaint.setColor(Color.GREEN);
      mPaint.setTextSize(30);
      mPaint.setTextAlign(Paint.Align.CENTER);
      mCameraFrameHeight = 0;
      mCameraFrameWidth = 0;
      JNIInterface.guiInitialise(this);
  }

  public void surfaceCreated(SurfaceHolder holder) {
  }

  public void surfaceDestroyed(SurfaceHolder holder) {
  }

  public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
  }

  public void requestDraw() {
    Canvas canvas = mHolder.lockCanvas();
    canvas.drawColor(Color.TRANSPARENT, Mode.CLEAR);
    mHolder.unlockCanvasAndPost(canvas);
  }

  public void setCameraSize(int width, int height) {
    if (width > 0 && height > 0) {
      mCameraFrameHeight = height;
      mCameraFrameWidth = width;
    }
  }
}
