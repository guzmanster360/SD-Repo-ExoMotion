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
    
    var bleManager = BLEManager()
    var storageManager = StorageManager()
    var awsManager = AWSManager()
    var userName: String?
    
    @IBOutlet weak var espConnect: UIButton!
    @IBOutlet weak var beginSessionButton: UIButton!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        configureUI()
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        self.view.endEditing(true) // ✅ Dismiss keyboard on screen tap
    }

    @IBAction func beginSessionPressed(_ sender: Any) {
        
        let fetchedResults = storageManager.retrieveUserData()
            for userStored in fetchedResults {
                if fetchedResults.count > 0 {
                    var userCheck = userStored.value(forKey: "userName") as? String
                    var userIDCheck = userStored.value(forKey: "userID") as? String
                    var sessionDescCheck = userStored.value(forKey: "sessionDescription") as? String
                    
                    if userCheck == "" || sessionDescCheck == "" || userIDCheck == ""{
                        let alert = UIAlertController(title: "Missing Data in Settings", message: "Please Enter a User Info.", preferredStyle: .alert)
                        // adds option to the alert
                        alert.addAction(UIAlertAction(title: "Ok", style: .default) { _ in
                        })
                        self.present(alert, animated: true, completion: nil)
                    }
                    else {
                        var sessionID = UUID().uuidString
                        storageManager.clearData()
                        awsManager.createS3Folder(userID: userIDCheck!, sessionID: sessionID)
                        storageManager.storeUser(userName: userCheck!, userID: userIDCheck!, session: sessionDescCheck!, sessionID: sessionID)
                        bleManager.startScanning()
                    }
                }
                else {
                        let alert = UIAlertController(title: "Missing Data in Settings", message: "Please Enter a User Info.", preferredStyle: .alert)
                        // adds option to the alert
                        alert.addAction(UIAlertAction(title: "Ok", style: .default) { _ in
                        })
                        self.present(alert, animated: true, completion: nil)
                    }
                }
    }
    
    @IBAction func bleConnect(_ sender: Any) {
        bleManager.startScanning()
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
