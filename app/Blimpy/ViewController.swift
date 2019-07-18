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

class ViewController: UIViewController, CocoaMQTTDelegate {
    var client: CocoaMQTT?
    var connected: Bool = false
    
    var jld: CDJoystickData?
    var jrd: CDJoystickData?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // setup joysticks
        setupLeftJoystick()
        setupRighttJoystick()
    
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
        joystick.trackingHandler = { joystickData in
            self.jld = joystickData
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
        joystick.trackingHandler = { joystickData in
            self.jrd = joystickData
            self.updateControls()
        }
        
        // add joystick
        view.addSubview(joystick)
    }
    
    var lastSend: Double = 0
    
    func updateControls() {
        // check data
        if jld == nil || jrd == nil {
            return
        }
        
        // get data
        let fwdBwd = Double(jrd!.velocity.y) * -1 // x
        let upDown = Double(jld!.velocity.y) * -1 // z
        // let leftRight = Double(jrd!.velocity.x) // y
        
        // calculate motor speeds
        let speeds = calcForce(x: fwdBwd, z: upDown)
        
        // get time
        let now = CACurrentMediaTime()
        
        // check rate if not all zero
        if speeds[0] != 0 || speeds[1] != 0 || speeds[2] != 0 || speeds[3] != 0 {
            if lastSend > now - 0.2 {
                return
            }
        }
        
        // set motor speeds
        setMotors(m1: speeds[0], m2: speeds[1], m3: speeds[2], m4: speeds[3])
        
        // set time
        lastSend = now
    }
    
    func clamp(v: Int) -> Int {
        // clamp to range
        let v = min(max(v, -255), 255)
        
        // reduce to zero
        if abs(v) < 50 {
            return 0
        }
        
        return v
    }
    
    func setMotors(m1: Double, m2: Double, m3: Double, m4: Double) {
        // get integers
        let mm1 = clamp(v: Int(m1))
        let mm2 = clamp(v: Int(m2))
        let mm3 = clamp(v: Int(m3))
        let mm4 = clamp(v: Int(m4))
        
        // encode speeds
        let speeds = [String(mm1), String(mm2), String(mm3), String(mm4)]
        let value = speeds.joined(separator: ",")
        
        // send speeds
        client?.publish("blimpy/mX", withString: value)
    }
    
    // CocoaMQTTDelegate
    
    func mqtt(_ mqtt: CocoaMQTT, didConnectAck ack: CocoaMQTTConnAck) {
        // return immediately if connection has been rejected
        if ack != .accept {
            return
        }
        
        // set flag
        connected = true
    }
    
    func mqtt(_ mqtt: CocoaMQTT, didPublishMessage message: CocoaMQTTMessage, id: UInt16) {}
    
    func mqtt(_ mqtt: CocoaMQTT, didPublishAck id: UInt16) {}
    
    func mqtt(_ mqtt: CocoaMQTT, didReceiveMessage message: CocoaMQTTMessage, id: UInt16) {
        // TODO: Handle message.
    }
    
    func mqtt(_ mqtt: CocoaMQTT, didSubscribeTopic topic: String) {}
    
    func mqtt(_ mqtt: CocoaMQTT, didUnsubscribeTopic topic: String) {}
    
    func mqttDidPing(_ mqtt: CocoaMQTT) {}
    
    func mqttDidReceivePong(_ mqtt: CocoaMQTT) {}
    
    func mqttDidDisconnect(_ mqtt: CocoaMQTT, withError err: Error?) {
        // set flag
        connected = false
    }
}
