package pt.b13h.castleofviana

import android.content.res.AssetManager
import android.graphics.Bitmap
import android.media.AudioAttributes
import android.media.AudioManager
import android.media.SoundPool
import android.os.Build
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import java.nio.ByteBuffer

class MainActivity : AppCompatActivity(), View.OnClickListener {
    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }

    private lateinit var soundPool : SoundPool
    private var sounds = IntArray(8)
    private var pixels = ByteArray(320 * 240 * 4)
    val bitmap: Bitmap = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888)
    private var running = false
    private external fun getPixelsFromNative(javaSide: ByteArray?)
    private external fun initAssets(assetManager: AssetManager?)
    private external fun sendCommand(cmd: Char)
    private external fun getSoundToPlay() : Int
    private external fun isOnMainMenu() : Int


    private fun initAudio() {
        soundPool = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            SoundPool.Builder().setAudioAttributes(AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_GAME)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build()).build()
        } else {
            SoundPool(5, AudioManager.STREAM_MUSIC, 0 )
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
        if (savedInstanceState == null ) {
            initAssets(resources.assets)
        }


        btnUp.setOnClickListener(this)
        btnDown.setOnClickListener(this)
        btnFire.setOnClickListener(this)
        btnPick.setOnClickListener(this)
        btnAim.setOnClickListener(this)
        btnLeft.setOnClickListener(this)
        btnRight.setOnClickListener(this)
        btnStart.setOnClickListener(this)

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
        Thread(Runnable {
            while (running) {
                runOnUiThread { redraw() }
                Thread.sleep(75)
            }
        }
        ).start()

        Thread(Runnable {
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

    override fun onBackPressed() {
        if (isOnMainMenu() != 0 ) {
            super.onBackPressed()
        } else {
            sendCommand('q')
        }
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

    override fun onClick(v: View) {
        var toSend = '.'
        when (v.id) {
            R.id.btnUp -> toSend = 'w'
            R.id.btnDown -> toSend = 's'
            R.id.btnLeft -> toSend = 'a'
            R.id.btnRight -> toSend = 'd'
            R.id.btnFire -> toSend = 'z'
            R.id.btnAim -> toSend = 'x'
            R.id.btnPick -> toSend = 'c'
            R.id.btnStart-> toSend = 'e'
        }
        sendCommand(toSend)
    }
}