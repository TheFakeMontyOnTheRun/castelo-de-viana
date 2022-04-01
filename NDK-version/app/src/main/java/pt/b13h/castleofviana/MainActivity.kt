package pt.b13h.castleofviana

import android.content.res.AssetManager
import android.graphics.Bitmap
import android.media.AudioAttributes
import android.media.AudioManager
import android.media.SoundPool
import android.os.Build
import android.os.Bundle
import android.view.MotionEvent
import android.view.View
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.AppCompatImageButton
import java.nio.ByteBuffer

class MainActivity : AppCompatActivity(), View.OnTouchListener {

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    private lateinit var imageView: ImageView
    private lateinit var btnRight: AppCompatImageButton
    private lateinit var btnLeft: AppCompatImageButton
    private lateinit var btnSword: AppCompatImageButton
    private lateinit var btnJump: AppCompatImageButton
    private lateinit var btnArrow: AppCompatImageButton
    private lateinit var btnDown: AppCompatImageButton
    private lateinit var btnUp: AppCompatImageButton
    private lateinit var soundPool: SoundPool
    private var sounds = IntArray(8)
    private var pixels = ByteArray(320 * 240 * 4)
    val bitmap: Bitmap = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888)
    private var running = false
    private external fun getPixelsFromNative(javaSide: ByteArray?)
    private external fun initAssets(assetManager: AssetManager?)
    private external fun sendCommand(cmd: Int)
    private external fun getSoundToPlay(): Int

    val KEY_UP = 1.shl(0)
    val KEY_RIGHT = 1.shl(1)
    val KEY_DOWN = 1.shl(2)
    val KEY_LEFT = 1.shl(3)
    val KEY_SWORD = 1.shl(4)
    val KEY_JUMP = 1.shl(5)
    val KEY_ARROW = 1.shl(6)
    val KEY_START = 1.shl(7)

    var keyState: Int = 0

    private fun initAudio() {
        soundPool = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            SoundPool.Builder().setAudioAttributes(
                AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_GAME)
                    .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                    .build()
            ).build()
        } else {
            SoundPool(5, AudioManager.STREAM_MUSIC, 0)
        }
/*
#define MENU_SELECTION_CHANGE_SOUND 0
#define STATE_CHANGE_SOUND 1
#define INFORMATION_ACQUIRED_SOUND 2
#define FAILED_TO_GET_INFORMATION_SOUND 3
#define PLAYER_GOT_DETECTED_SOUND 4
#define PLAYER_FIRING_GUN 5
#define ENEMY_FIRING_GUN 6
#define PLAYER_GET_HURT_SOUND 7


        sounds[0] = soundPool.load(this, R.raw.menu_move, 1)
        sounds[1] = soundPool.load(this, R.raw.menu_select, 1)
        sounds[2] = soundPool.load(this, R.raw.gotclue, 1)
        sounds[3] = soundPool.load(this, R.raw.detected, 1)
        sounds[4] = soundPool.load(this, R.raw.detected2, 1)
        sounds[5] = soundPool.load(this, R.raw.menu_select, 1)
        sounds[6] = soundPool.load(this, R.raw.menu_select, 1)
        sounds[7] = soundPool.load(this, R.raw.hurt, 1)
*/
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)
        if (savedInstanceState == null) {
            initAssets(resources.assets)
        }

        btnUp = findViewById(R.id.btnUp)
        btnUp.setOnTouchListener(this)
        btnDown = findViewById(R.id.btnDown)
        btnDown.setOnTouchListener(this)
        btnArrow = findViewById(R.id.btnArrow)
        btnArrow.setOnTouchListener(this)
        btnJump = findViewById(R.id.btnJump)
        btnJump.setOnTouchListener(this)
        btnSword = findViewById(R.id.btnSword)
        btnSword.setOnTouchListener(this)
        btnLeft = findViewById(R.id.btnLeft)
        btnLeft.setOnTouchListener(this)
        btnRight = findViewById(R.id.btnRight)
        btnRight.setOnTouchListener(this)
        imageView = findViewById(R.id.imageView)
        imageView.setOnTouchListener(this)

        initAudio()
        imageView.setImageBitmap(bitmap)
    }

    override fun onDestroy() {

        soundPool.release()
        super.onDestroy()
    }

    override fun onResume() {
        super.onResume()
        running = true
        Thread {
            while (running) {
                sendCommand(keyState)
                runOnUiThread { redraw() }
                Thread.sleep(75)
            }
        }.start()

        Thread({
            /*
            while (running) {
                when (val sound = getSoundToPlay()) {
                    0, 1, 2, 3, 4, 5, 6, 7, 8 -> soundPool.play(sounds[sound], 1f, 1f, 0, 0, 1f)
                }
                Thread.sleep(10)
            }

             */
        }
        ).start()
    }

    override fun onPause() {
        super.onPause()
        running = false
    }

    private fun redraw() {
        getPixelsFromNative(pixels)
        bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(pixels))
        imageView.invalidate()
    }

    override fun onTouch(v: View?, event: MotionEvent?): Boolean {
        if (event != null && v != null) {
            if (event.action == MotionEvent.ACTION_DOWN) {
                when (v.id) {
                    R.id.btnUp -> keyState = (keyState or KEY_UP)
                    R.id.btnDown -> keyState = (keyState or KEY_DOWN)
                    R.id.btnLeft -> keyState = (keyState or KEY_LEFT)
                    R.id.btnRight -> keyState = (keyState or KEY_RIGHT)
                    R.id.btnSword -> keyState = (keyState or KEY_SWORD)
                    R.id.btnJump -> keyState = (keyState or KEY_JUMP)
                    R.id.btnArrow -> keyState = (keyState or KEY_ARROW)
                    R.id.imageView -> keyState = (keyState or KEY_START)
                }
            } else if (event.action == MotionEvent.ACTION_UP) {
                when (v.id) {
                    R.id.btnUp -> keyState = (keyState and KEY_UP.inv())
                    R.id.btnDown -> keyState = (keyState and KEY_DOWN.inv())
                    R.id.btnLeft -> keyState = (keyState and KEY_LEFT.inv())
                    R.id.btnRight -> keyState = (keyState and KEY_RIGHT.inv())
                    R.id.btnSword -> keyState = (keyState and KEY_SWORD.inv())
                    R.id.btnJump -> keyState = (keyState and KEY_JUMP.inv())
                    R.id.btnArrow -> keyState = (keyState and KEY_ARROW.inv())
                    R.id.imageView -> keyState = (keyState and KEY_START.inv())
                }
            }
        }
        return true
    }
}