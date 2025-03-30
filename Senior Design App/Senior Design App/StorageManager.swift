//
//  StorageManager.swift
//  Senior Design App
//
//  Created by Victor Guzman on 3/27/25.
//

import UIKit
import CoreData

let appDelegate = UIApplication.shared.delegate as! AppDelegate
let context = appDelegate.persistentContainer.viewContext

class StorageManager {
    
    // store data in core data based on user input
    func storeUser(userName:String, userID:String, session:String, sessionID:String) {
        // define database entity being searched for
        let user = NSEntityDescription.insertNewObject(forEntityName: "User", into: context)
        // set all attributes defined in core data based on what the user inputted
        user.setValue(sessionID, forKey: "sessionID")
        user.setValue(session, forKey: "sessionDescription")
        user.setValue(userName, forKey: "userName")
        user.setValue(userID, forKey: "userID")
        // commit the changes
        saveContext()
    }
    
    func retrieveUserData() -> [NSManagedObject] {
    
        // requst only user entities
        let request = NSFetchRequest<NSFetchRequestResult>(entityName: "User")
        // fetch the results in an array
        var fetchedResults:[NSManagedObject]?
        // attempt to fetch results while preparing for errors
        do {
            // attempt to get the data based on defined request
            try fetchedResults = context.fetch(request) as? [NSManagedObject]
        } catch {
            // if attempt fails print an error instead of crashing
            print("Error occurred while retrieving data")
            abort()
        }
        // return an array of the data desired
        return(fetchedResults)!
    }
    
    // retrieve the pizza the user wants to delete and delete it takes all pizza attributes
    func clearData() {
        // define request for pizza entity
        let request = NSFetchRequest<NSFetchRequestResult>(entityName: "User")
        // define array of pizzas to be requested
        var fetchedResults:[NSManagedObject]
        // attempt to fetch results based on request
        do {
            // try to populate results array based on request parameters
            try fetchedResults = context.fetch(request) as! [NSManagedObject]
            // checks if there is data that was fetched
            if fetchedResults.count > 0 {
                // nested if statements checking each property of the current pizza entity with the inputed properties to ensure the right pizza is deleted from core data
                for result in fetchedResults {
                    context.delete(result)
                }
            }
            // update core data and complete the deletion
            saveContext()
        // handle any errors to prevent app crashing
        } catch {
            print("Error occurred while clearing data")
            abort()
        }
    }
    
    func saveContext () {
        // check if any data has been staged for being committed
        if context.hasChanges {
            do {
                // attempt to add data
                try context.save()
                // if there is an error adding data print the error below
            } catch {
                let nserror = error as NSError
                fatalError("Unresolved error \(nserror), \(nserror.userInfo)")
            }
        }
    }
}
