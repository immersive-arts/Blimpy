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

class ViewController: UIViewController, CocoaMQTTDelegate {
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
    
    var lastSpeeds: Array<Int> = [0, 0, 0, 0]
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
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
    
        // create client
        client = CocoaMQTT(clientID: "app", host: "10.128.96.191", port: 8883)
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
        let fx = jry * -1
        let fz = jly * -1
        let mz = jrx * -1
        var mx = 0.0
        
        // use motion if enabled
        if motion {
           mx = pitch
        }
        
        // calculate motor speeds
        let speeds = Model.calculate(fx: fx, fz: fz, mx: mx, mz: mz)
        
        // get hash
        let dist = distance(a: speeds, b: lastSpeeds)
        
        // skip if distance didn't change much
        if abs(dist) < 50 {
            return
        }
        
        // set last speeds
        lastSpeeds = speeds
        
        // map ints to strings
        let stringSpeeds = speeds.map { (v: Int) -> String in
            return String(v)
        }

        // encode message
        let message = stringSpeeds.joined(separator: ",")
        
        // send message
        client?.publish("blimpy/motors", withString: message, qos: .qos0, retained: false)
    }
    
    @IBAction func motionSwitch(_ sender: UISwitch) {
        // set flag
        motion = (sender.isOn)
    }
    
    // Utilities
    
    func distance(a: Array<Int>, b: Array<Int>) -> Double {
        var total = 0
        var diff = 0
        for (i, _) in a.enumerated() {
            diff = b[i] - a[i];
            total += diff * diff;
        }
        return sqrt(Double(total))
    }
    
    // CocoaMQTTDelegate
    
    func mqtt(_ mqtt: CocoaMQTT, didConnectAck ack: CocoaMQTTConnAck) {}
    func mqtt(_ mqtt: CocoaMQTT, didPublishMessage message: CocoaMQTTMessage, id: UInt16) {}
    func mqtt(_ mqtt: CocoaMQTT, didPublishAck id: UInt16) {}
    func mqtt(_ mqtt: CocoaMQTT, didReceiveMessage message: CocoaMQTTMessage, id: UInt16) {}
    func mqtt(_ mqtt: CocoaMQTT, didSubscribeTopic topic: String) {}
    func mqtt(_ mqtt: CocoaMQTT, didUnsubscribeTopic topic: String) {}
    func mqttDidPing(_ mqtt: CocoaMQTT) {}
    func mqttDidReceivePong(_ mqtt: CocoaMQTT) {}
    func mqttDidDisconnect(_ mqtt: CocoaMQTT, withError err: Error?) {}
}
