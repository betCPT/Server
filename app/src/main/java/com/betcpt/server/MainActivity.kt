package com.betcpt.server

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.betcpt.server.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val greet = getGreeting()
        val sum = addNumbers(7, 3)

        val message = "$greet\n7 + 3 = $sum"
        val textView = TextView(this)
        textView.text = message
        setContentView(textView)

    }

    /**
     * A native method that is implemented by the 'server' native library,
     * which is packaged with this application.
     */

    external fun getGreeting(): String
    external fun addNumbers(a: Int, b: Int): Int

    companion object {
        // Used to load the 'server' library on application startup.
        init {
            System.loadLibrary("server")
        }
    }
}