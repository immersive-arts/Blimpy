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
    
    var lastSpeeds: Array<Int> = [0, 0, 0, 0]
    
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
        // get axes
        let fwdBwd = Double(jry) * -1 // x
        let upDown = Double(jly) * -1 // z
        let leftRight = Double(jrx) * -1 // mz
        
        // calculate motor speeds
        let speeds = Model.calculate(fx: fwdBwd, fz: upDown, mz: leftRight)
        
        // get hash
        let dist = distance(a: speeds, b: lastSpeeds)
        
        // skip if distance didn't change much
        if abs(dist) < 30 {
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
        client?.publish("blimpy/mX", withString: message, qos: .qos0, retained: false)
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
