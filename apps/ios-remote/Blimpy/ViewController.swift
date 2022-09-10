//
//  ViewController.swift
//  Blimpy
//
//  Created by Joël Gähwiler on 18.07.19.
//  Copyright © 2019 Joël Gähwiler. All rights reserved.
//

import UIKit
import CocoaMQTT
import CDJoystick
import CoreMotion

class ViewController: UIViewController, CocoaMQTTDelegate, UITableViewDataSource, UITableViewDelegate {
    var baseTopics: Set<String> = []
    var baseTopic = ""
    
    var manager: CMMotionManager = CMMotionManager()
    var client: CocoaMQTT?
    
    var jlx: Double = 0
    var jly: Double = 0
    var jrx: Double = 0
    var jry: Double = 0
    var roll: Double = 0
    var yaw: Double = 0
    var pitch: Double = 0
    
    var motion: Bool = false
    
    var moveSensitivity: Double = 0.5
    var turnSensitivity: Double = 0.5
    
    var lastSpeeds: Array<Double> = [0, 0, 0, 0, 0, 0]
    
    @IBOutlet weak var table: UITableView!
    
    @IBOutlet weak var battery: UIProgressView!
    @IBOutlet weak var wifi: UIProgressView!
    @IBOutlet weak var status: UIView!
    
    @IBOutlet weak var batteryLabel: UILabel!
    @IBOutlet weak var wifiLabel: UILabel!
    @IBOutlet weak var systemLabel: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // set initial values
        battery.progress = 0
        wifi.progress = 0
        
        // setup status
        status.layer.cornerRadius = 16
        status.backgroundColor = UIColor.red
        
        // setup joysticks
        setupLeftJoystick()
        setupRighttJoystick()
        
        // set motion interval
        manager.deviceMotionUpdateInterval = 0.05
        
        // listen to motion
        manager.startDeviceMotionUpdates(to: .main) { (data, error) in
            if let data = data {
                self.yaw = data.attitude.yaw
                self.roll = data.attitude.roll
                self.pitch = data.attitude.pitch
                self.updateControls()
            }
        }
        
        // get settings
        let host = UserDefaults.standard.string(forKey: "BrokerHost") ?? ""
        let port = UserDefaults.standard.integer(forKey: "BrokerPort")
        let username = UserDefaults.standard.string(forKey: "BrokerUsername")
        let password = UserDefaults.standard.string(forKey: "BrokerPassword")
        baseTopic = UserDefaults.standard.string(forKey: "BaseTopic") ?? ""
        
        // create client
        client = CocoaMQTT(clientID: "app", host: host, port: UInt16(port))
        client!.username = username
        client!.password = password
        client!.delegate = self
        client!.autoReconnect = true
        client!.autoReconnectTimeInterval = 1
        
        // connect to broker
        client!.connect()
    }
    
    func setupLeftJoystick() {
        // prepare joystick
        let joystick = CDJoystick()
        joystick.frame = CGRect(x: 50, y: view.center.y - 100, width: 200, height: 200)
        joystick.backgroundColor = .clear
        joystick.substrateColor = .lightGray
        joystick.substrateBorderColor = .gray
        joystick.substrateBorderWidth = 1.0
        joystick.stickSize = CGSize(width: 100, height: 100)
        joystick.stickColor = .darkGray
        joystick.stickBorderColor = .black
        joystick.stickBorderWidth = 2.0
        joystick.fade = 0.5
        
        // set handler
        joystick.trackingHandler = { data in
            self.jlx = Double(data.velocity.x)
            self.jly = Double(data.velocity.y)
            self.updateControls()
        }
        
        // add joystick
        view.addSubview(joystick)
    }
    
    func setupRighttJoystick() {
        // prepare joystick
        let joystick = CDJoystick()
        joystick.frame = CGRect(x: view.bounds.width - 250, y: view.center.y - 100, width: 200, height: 200)
        joystick.backgroundColor = .clear
        joystick.substrateColor = .lightGray
        joystick.substrateBorderColor = .gray
        joystick.substrateBorderWidth = 1.0
        joystick.stickSize = CGSize(width: 100, height: 100)
        joystick.stickColor = .darkGray
        joystick.stickBorderColor = .black
        joystick.stickBorderWidth = 2.0
        joystick.fade = 0.5
        
        // set handler
        joystick.trackingHandler = { data in
            self.jrx = Double(data.velocity.x)
            self.jry = Double(data.velocity.y)
            self.updateControls()
        }
        
        // add joystick
        view.addSubview(joystick)
    }
    
    func updateControls() {
        // prepare values
        let fx = jry * -1 * moveSensitivity
        let fy = jlx * -1 * moveSensitivity
        let fz = jly * -1 * moveSensitivity
        var mx = 0.0
        var my = 0.0
        let mz = jrx * -1 * turnSensitivity
        
        // use motion if enabled
        if motion {
            mx = pitch
            my = roll
        }
        
        // calculate motor speeds
        let speeds = [fx, fy, fz, mx, my, mz]
        
        // get difference
        let diff = difference(a: speeds, b: lastSpeeds)
        
        // skip if difference didn't change much
        if abs(diff) < 0.05 {
            return
        }
        
        // set last speeds
        lastSpeeds = speeds
        
        // map ints to strings
        let stringSpeeds = speeds.map { (v: Double) -> String in
            return String(v)
        }

        // encode message
        let message = stringSpeeds.joined(separator: ",")
        
        // send message
        client?.publish(baseTopic + "/forces", withString: message, qos: .qos0, retained: false)
    }
    
    @IBAction func motionSwitch(_ sender: UISwitch) {
        // set flag
        motion = (sender.isOn)
    }
    
    @IBAction func moveSensitivitySlider(_ sender: UISlider) {
        // set value
        moveSensitivity = Double(sender.value)
    }
    
    @IBAction func turnSensitivitySlider(_ sender: UISlider) {
        // set value
        turnSensitivity = Double(sender.value)
    }
    
    // Utilities
    
    func difference(a: Array<Double>, b: Array<Double>) -> Double {
        var total = 0.0
        for (i, _) in a.enumerated() {
            total += abs(b[i] - a[i]);
        }
        return total
    }
    
    // CocoaMQTTDelegate
    
    func mqtt(_ mqtt: CocoaMQTT, didConnectAck ack: CocoaMQTTConnAck) {
        if ack == .accept {
            status.backgroundColor = UIColor.green
            client?.subscribe("#", qos: .qos0)
        }
    }
    
    func mqtt(_ mqtt: CocoaMQTT, didReceiveMessage message: CocoaMQTTMessage, id: UInt16) {
        // get data
        let data = String(data: Data(message.payload), encoding: .utf8)
        
        // check generic heartbeat
        if message.topic.hasSuffix("/naos/heartbeat") {
            // get base topic
            let bt = String(message.topic[..<message.topic.index(message.topic.endIndex, offsetBy: -15)])
            
            // add and reload table
            if !baseTopics.contains(bt) {
                baseTopics.insert(bt)
                table.reloadData()
            }
        }
        
        // check exact hearbeat
        if message.topic == baseTopic + "/naos/heartbeat" {
            // get info
            let info = data?.split(separator: ",")
            
            // map charge to percentage
            let ch = (Float(info?[6] ?? "0") ?? 0) * 100
            
            // set battery level and label
            battery.progress = ch / 100
            batteryLabel.text = String(format: "BATTERY %.0f%%", ch)
            
            // map signal strength to percentage
            var ss = (100 - ((Float(info?[7] ?? "0") ?? 0) * -1)) * 2
            if ss > 100 {
                ss = 100
            } else if ss < 0 {
                ss = 0
            }
            
            // set wifi strength and label
            wifi.progress = ss / 100
            wifiLabel.text = String(format: "WIFI %.0f%%", ss)
        }
        
        // check exact battery
        if message.topic == baseTopic + "/battery" {
            // get info
            let info = data?.split(separator: ",")
            
            // get voltages
            let voltage = Float(info?[1] ?? "0") ?? 0
            let avgVoltage = Float(info?[2] ?? "0") ?? 0
            
            // get currents
            let current = Float(info?[3] ?? "0") ?? 0
            let avgCurrent = Float(info?[4] ?? "0") ?? 0
            
            // set label
            systemLabel.text = String(format: "%.3fV (%.3fV) / %.3fA (%.3fA)", voltage, avgVoltage, current, avgCurrent)
        }
    }
    
    func mqtt(_ mqtt: CocoaMQTT, didSubscribeTopics success: NSDictionary, failed: [String]) {}
    func mqtt(_ mqtt: CocoaMQTT, didUnsubscribeTopics topics: [String]) {}
    func mqtt(_ mqtt: CocoaMQTT, didPublishMessage message: CocoaMQTTMessage, id: UInt16) {}
    func mqtt(_ mqtt: CocoaMQTT, didPublishAck id: UInt16) {}
    func mqtt(_ mqtt: CocoaMQTT, didSubscribeTopic topic: String) {}
    func mqtt(_ mqtt: CocoaMQTT, didUnsubscribeTopic topic: String) {}
    func mqttDidPing(_ mqtt: CocoaMQTT) {}
    func mqttDidReceivePong(_ mqtt: CocoaMQTT) {}
    
    func mqttDidDisconnect(_ mqtt: CocoaMQTT, withError err: Error?) {
        status.backgroundColor = UIColor.red
    }
    
    /* UITableViewDataSource */
    
    func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return baseTopics.count
    }
    
    func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        // get celel
        let cell = tableView.dequeueReusableCell(withIdentifier: "DroneCell", for: indexPath)

        // set label text
        cell.textLabel!.text = baseTopics[baseTopics.index(baseTopics.startIndex, offsetBy: indexPath.row)]

        return cell
    }
    
    
    /* UITableViewDelegate */
    
    func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        // set base topic
        baseTopic = baseTopics[baseTopics.index(baseTopics.startIndex, offsetBy: indexPath.row)]
    }
}
