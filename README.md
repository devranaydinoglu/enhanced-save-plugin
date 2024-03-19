# Enhanced Save System

Enhanced Save System (ESS) is an easy-to-use and blueprints-compatible save system which offers saving and loading.  
ESS is available in UE versions 5.0 - 5.3 with versions 5.1 - 5.3 still receiving new updates.  
ESS can be purchased on the Unreal Engine Marketplace: https://www.unrealengine.com/marketplace/en-US/product/enhanced-save-system

## Terminology

There are two types of actors in ESS:

- Unique actor: an actor with a GUID assigned in its ESSUniqueSavableComponent.
- World actor: an actor without an ESSUniqueSavableComponent (and therefore without a GUID).

## Getting Started

#### 1. SaveGame Object

ESS requires you to inherit your SaveGame Object class from ESSSaveGame. This is required for storing custom data.

#### 2. Make actors, objects, and actor components detectable by ESS

In order for actors, objects, and actor components to be considered for saving and loading by ESS, they need to implement the ESSSavableInterface interface.  
An actor becomes a unique actor by adding the ESSUniqueSavableComponent to it. Unique actors are actors with a GUID which can be assigned in the ESSUniqueSavableComponent. These actors will not be saved by the Save/LoadActorsInWorld functions. Instead, they need to be saved and loaded with SaveActor and LoadActor functions.

#### 3. Mark variables

Variables that need to be saved and loaded need to be marked as "SaveGame". In blueprints, this can be done in the variable's details panel under advanced options. In C++, the specifier `SaveGame` needs to be added to the variable's `UPROPERTY()`.

#### 4. Save and Load

Now that actors, objects, and actor components can be detected by ESS, it's time to actually save/load them. Simply call any of the below functions based on your needs!

Available functions:
- `SaveActorsInWorld` - Save variables that are marked as "SaveGame" of all world actors and their actor components.
- `LoadActorsInWorld` - Load variables that are marked as "SaveGame" of all world actors and their actor components.
- `SaveActor` - Save an actor's and its component's variables that are marked as "SaveGame". ESS will automatically save the actor as a world actor or unique actor depending on if it has an ESSUniqueSavableComponent.
- `LoadActor` - Load an actor's and its component's variables that are marked as "SaveGame". **IMPORTANT:** Actor needs to be in the world when this function is called. If the GUID of the ESSSavableComponent of an actor has been changed, it needs to be saved again. Its old data which uses the old GUID cannot be loaded with the new GUID.
- `SaveObject` - Save an object's variables that are marked as "SaveGame". Should only be used to save objects that aren't actors.
- `LoadObject` - Load an object's variables that are marked as "SaveGame". Should only be used to load objects that aren't actors. **IMPORTANT:** Object needs to be in the world when this function is called.

## Classes

#### ESSBlueprintLibrary

Holds all functions related to saving and loading.

#### ESSSavableInterface

Used to detect savable actors, objects, and actor components.

Overridable  functions:
- `PreSaveGame` - Overridable  function which gets called before saving data.
- `PostLoadGame` - Overridable  function which gets called after loading data.

#### ESSUniqueSavableComponent

Used to have uniquely identifiable actors based on a GUID. These special actors with a GUID will be saved separately from actors which don’t have the “GUID” property specified.  
The GUID property is only considered/utilized when using the following save/load functions: `SaveActor`, `LoadActor`.
Special actors are saved in a map to make them easily searchable with the GUID. This has been introduced so that actors can be uniquely identified and to keep them separated from world actors. A use-case for this is if you want to save or load only a specific actor and keep it separated from world actors.

#### ESSSaveGame

Used to store and load all save data. World actors, unique actors, and objects are stored in separate containers. This means that, e.g., `LoadActorsInWorld` won't load data of unique actors. `LoadActor` *will* load data of both world actors and unique actors.

The following actor data is stored:
- Name - The actor's `FName`.
- Class - The actor's class.
- Transform - The actor's world transform.
- Savable variables - The actor's variables which are marked as "SaveGame".

The following object data is stored:
- Name - The object's `FName`.
- Class - The object's class.
- Savable variables - The object's variables which are marked as "SaveGame".
