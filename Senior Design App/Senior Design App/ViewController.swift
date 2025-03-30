//
//  ViewController.swift
//  Senior Design App
//
//  Created by Victor Guzman on 3/2/25.
//

import UIKit
import CoreBluetooth
import CoreData

// add session ID
// add user ID input and decoding

class ViewController: UIViewController {
    
    var bleManager: BLEManager?
    var storageManager: StorageManager?
    var userName: String?
    
    @IBOutlet weak var espConnect: UIButton!
    @IBOutlet weak var beginSessionButton: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        configureUI()
        //        Timer.scheduledTimer(timeInterval: 1.0, target: self, selector: #selector(getter: dataLabel), userInfo: nil, repeats: true)
        // Do any additional setup after loading the view.
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        self.view.endEditing(true) // ✅ Dismiss keyboard on screen tap
    }

    @IBAction func beginSessionPressed(_ sender: Any) {
        
        let fetchedResults = storageManager!.retrieveUserData()
        // iterate through the pizzas in core data
        bleManager!.startScanning()
    }
    
    @IBAction func bleConnect(_ sender: Any) {
        bleManager!.startScanning()
    }
    
    func configureUI() {
        espConnect.layer.cornerRadius = 10  // ✅ Rounded corners
        espConnect.layer.borderWidth = 2
        espConnect.layer.borderColor = UIColor.white.cgColor
        
        beginSessionButton.layer.cornerRadius = 20  // ✅ Rounded corners
        beginSessionButton.layer.borderWidth = 3
        beginSessionButton.layer.borderColor = UIColor.black.cgColor
    }

}
