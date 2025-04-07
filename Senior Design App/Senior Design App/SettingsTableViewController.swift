//
//  SettingsTableViewController.swift
//  Senior Design App
//
//  Created by Victor Guzman on 3/25/25.
//

import UIKit

class SettingsTableViewController: UITableViewController {
    
    var storageManager = StorageManager()
    var awsManager = AWSManager()
    
    @IBOutlet weak var userNameField: UITextField!
    @IBOutlet weak var userIDLabel: UILabel!
    @IBOutlet weak var sessionDescriptionField: UITextField!
    @IBOutlet weak var sessionIDLabel: UILabel!
    
    var user: String = ""
    var userID: String = ""
    var session: String = ""
    var sessionID: String = ""

    override func viewDidLoad() {
        super.viewDidLoad()
        user = getStoredData()[0]
        userID = getStoredData()[1]
        session = getStoredData()[2]
        sessionID = getStoredData()[3]
        userNameField.text = user
        userIDLabel.text = userID
        sessionDescriptionField.text = session
        sessionIDLabel.text = sessionID
//        tableView.register(UITableViewCell.self, forCellReuseIdentifier: "cell")
    }
    
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        self.view.endEditing(true)
    }
    
    @IBAction func saveDataPressed(_ sender: Any) {
        
        if userNameField.text == "" {
            let alert = UIAlertController(title: "Missing User Name", message: "Please Enter a User Name ", preferredStyle: .alert)
            // adds option to the alert
            alert.addAction(UIAlertAction(title: "Ok", style: .default) { _ in
            })
            self.present(alert, animated: true, completion: nil)
        }
        else if sessionDescriptionField.text == "" {
            let alert = UIAlertController(title: "Missing Description", message: "Please Add a Session Description", preferredStyle: .alert)
            // adds option to the alert
            alert.addAction(UIAlertAction(title: "Ok", style: .default) { _ in
            })
            self.present(alert, animated: true, completion: nil)
        }
        else if userNameField.text == user {
            storageManager.clearData()
            sessionID = UUID().uuidString
            userIDLabel.text = userID
            sessionIDLabel.text = sessionID
            
            awsManager.createS3Folder(userID: userID, sessionID: sessionID)
            
            storageManager.storeUser(userName: userNameField.text!, userID: userID, session: sessionDescriptionField.text!, sessionID: sessionID)
        }
        else {
            storageManager.clearData()
            userID = UUID().uuidString
            sessionID = UUID().uuidString
            userIDLabel.text = userID
            sessionIDLabel.text = sessionID
            
            awsManager.createS3Folder(userID: userID, sessionID: sessionID)
            
            storageManager.storeUser(userName: userNameField.text!, userID: userID, session: sessionDescriptionField.text!, sessionID: sessionID)

        }
        view.endEditing(true)
    }
    
    @IBAction func newSessionButtonPressed(_ sender: Any) {
      
    }
    
    func getStoredData() -> [String] {
        var output: [String] = []
        let fetchedResults = storageManager.retrieveUserData()
            for userStored in fetchedResults {
                if fetchedResults.count > 0 {
                    output.append((userStored.value(forKey: "userName") as? String)!)
                    output.append((userStored.value(forKey: "userID") as? String)!)
                    output.append((userStored.value(forKey: "sessionID") as? String)!)
                    output.append((userStored.value(forKey: "sessionDescription") as? String)!)
                    return output
                }
            }
        return output
    }
}
