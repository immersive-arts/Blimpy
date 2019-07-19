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
    
    var jlx: CGFloat = 0
    var jly: CGFloat = 0
    var jrx: CGFloat = 0
    var jry: CGFloat = 0
    
    var lastSend: Double = 0
    
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
        joystick.trackingHandler = { data in
            self.jlx = data.velocity.x
            self.jly = data.velocity.y
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
            self.jrx = data.velocity.x
            self.jry = data.velocity.y
            self.updateControls()
        }
        
        // add joystick
        view.addSubview(joystick)
    }
    
    func updateControls() {
        // get data
        let fwdBwd = Double(jrx) * -1 // x
        let upDown = Double(jly) * -1 // z
        let leftRight = Double(jrx) * -1 // mz
        
        // calculate motor speeds
        let speeds = calcForce(fx: fwdBwd, fz: upDown, mz: leftRight)
        
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
        // clamp to duty cycle range
        let v = min(max(v, -255), 255)
        
        // reduce low values to zero
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
