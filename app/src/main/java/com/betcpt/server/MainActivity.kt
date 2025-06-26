package com.betcpt.server

import android.app.AlertDialog
import android.graphics.Color
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.betcpt.server.databinding.ActivityMainBinding

import android.widget.*
class MainActivity : AppCompatActivity() {

    private lateinit var clientListView: ListView
    private lateinit var commandInput: EditText
    private lateinit var sendToSelectedBtn: Button
    private lateinit var sendToAllBtn: Button
    private lateinit var clientCountLabel: TextView
    private lateinit var adapter: ArrayAdapter<String>

    private val connectedClients = mutableListOf<String>()
    private val clientNames = mutableMapOf<String, String>() // IP -> renamed

    companion object {
        init {
            System.loadLibrary("server")
        }
    }

    external fun startServer()
    external fun sendCommandTo(index: Int, command: String)
    external fun sendCommandToAll(command: String)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        clientListView = findViewById(R.id.clientListView)
        commandInput = findViewById(R.id.commandInput)
        sendToSelectedBtn = findViewById(R.id.sendToSelectedBtn)
        sendToAllBtn = findViewById(R.id.sendToAllBtn)
        clientCountLabel = findViewById(R.id.clientCountLabel)

        adapter = ArrayAdapter(this, R.layout.item_client, R.id.clientItem, connectedClients)
        clientListView.adapter = adapter
        clientListView.setOnItemLongClickListener { _, _, position, _ ->
            val oldName = connectedClients[position]
            val input = EditText(this).apply {
                setText(oldName)
                setTextColor(Color.WHITE)
                setBackgroundColor(Color.DKGRAY)
            }

            AlertDialog.Builder(this)
                .setTitle("Rename Client")
                .setView(input)
                .setPositiveButton("Save") { _, _ ->
                    val newName = input.text.toString()
                    val originalIp = clientNames.entries.find { it.value == oldName }?.key ?: oldName
                    clientNames[originalIp] = newName
                    connectedClients[position] = newName
                    sortClients()
                    adapter.notifyDataSetChanged()
                }
                .setNegativeButton("Cancel", null)
                .show()

            true
        }

        clientListView.choiceMode = ListView.CHOICE_MODE_MULTIPLE

        clientListView.setOnItemClickListener { _, _, _, _ -> updateSelectionCount() }

        sendToSelectedBtn.setOnClickListener {
            val cmd = commandInput.text.toString()
            val checked = clientListView.checkedItemPositions
            var sent = 0
            for (i in 0 until connectedClients.size) {
                if (checked[i]) {
                    sendCommandTo(i, cmd)
                    sent++
                }
            }
            Toast.makeText(this, "Sent to $sent client(s)", Toast.LENGTH_SHORT).show()
        }

        sendToAllBtn.setOnClickListener {
            val cmd = commandInput.text.toString()
            sendCommandToAll(cmd)
            Toast.makeText(this, "Broadcasted command", Toast.LENGTH_SHORT).show()
        }

        startServer()
    }
    private fun sortClients() {
        connectedClients.sortBy { it.lowercase() }
    }

    private fun updateSelectionCount() {
        val checked = clientListView.checkedItemPositions
        var count = 0
        for (i in 0 until connectedClients.size) {
            if (checked[i]) count++
        }
        clientCountLabel.text = "$count client(s) selected"
    }

    fun onClientConnected(clientDesc: String) {
        runOnUiThread {
            val name = clientNames[clientDesc] ?: clientDesc
            connectedClients.add(name)
            sortClients()
            adapter.notifyDataSetChanged()
            clientCountLabel.text = "${connectedClients.size} clients connected"
        }
    }

    fun onClientDisconnected(index: Int) {
        runOnUiThread {
            if (index in connectedClients.indices) {
                connectedClients.removeAt(index)
                adapter.notifyDataSetChanged()
                clientCountLabel.text = "${connectedClients.size} clients connected"
            }
        }
    }
}
