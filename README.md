# StoneFramework
It’s an application framework. Our purpose is an stable and easy-use framework.
Stable is the most important purpose. When we face an choice between speed and stable in design, we choose stable.
You will found this framework has brief interface and good coding rules. It’s easy to read and use.
There are some modules in this framework:
## Base
Include common definition, guard, smart pointer, mutex etc.
It a base for other module.
## Event
Include the event’s base definition. All events posted or sent in this framework are inherited from this base event class.
## Thread
Just an simple encapsulation of the pthread, but it’s easier to use.
## Tools
Include the string tools and the data items tool. Very useful module.
The string tool is used to convert data between string, int and double.
The data items is used to serialize the structure to string. It very useful in RPC.
## Net
Include the server, client and event’s definition.
It's easy to use server and client to implement RPC.
## Core
Include the application’s event loop and base class definition.
