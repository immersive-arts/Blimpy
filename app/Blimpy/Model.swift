//
//  Model.swift
//  Blimpy
//
//  Created by Joël Gähwiler on 18.07.19.
//  Copyright © 2019 Joël Gähwiler. All rights reserved.
//

import Foundation

// max force, max torque and inverse matrix calculated by calc.py
let FORCE_MAX = 721.24891681
let TORQUE_MAX = 360.6244584051392
let MODEL = [[ 0.35355339, -0.35355339,  0.70710678],
             [ 0.35355339,  0.35355339,  0.70710678],
             [ 0.35355339,  0.35355339, -0.70710678],
             [ 0.35355339, -0.35355339, -0.70710678]]

func calcForce(fx: Double, fz: Double, mz: Double) -> Array<Double> {
    // get forces
    let ffx = fx * FORCE_MAX
    let ffz = fz * FORCE_MAX
    
    // get torque
    let mmz = mz * TORQUE_MAX
    
    // get motor duty cycles
    let m1 = MODEL[0][0] * ffx + MODEL[0][1] * ffz + MODEL[0][2] * mmz
    let m2 = MODEL[1][0] * ffx + MODEL[1][1] * ffz + MODEL[1][2] * mmz
    let m3 = MODEL[2][0] * ffx + MODEL[2][1] * ffz + MODEL[2][2] * mmz
    let m4 = MODEL[3][0] * ffx + MODEL[3][1] * ffz + MODEL[3][2] * mmz
    
    return [m1, m2, m3, m4]
}
