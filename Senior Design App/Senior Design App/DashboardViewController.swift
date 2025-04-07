//
//  DashboardViewController.swift
//  Senior Design App
//
//  Created by Victor Guzman on 3/23/25.
//

import UIKit
import Charts


class DashboardViewController: UIViewController {
    var data = [1, 2, 3, 4, 5, 7, 8]

    override func viewDidLoad() {
        
        let timer = Timer.publish(every: 0.05, on: .main, in: .common).autoconnect()
        
        Chart(Array(data.enumerated()), id: \.0) { index, magnitude in
            BarMark (
                x: .value("Time", String(index)),
                y: .value("value", magnitude)
            )
        }
    }
}
