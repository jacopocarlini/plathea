import React, {
    Component
} from 'react';
import axios from 'axios';
import Item from "./Item";
import Room from "./Room";
import Person from "./Person";
import FileUpload from "./FileUpload";
import "./App.css";


class App extends React.Component {

    constructor() {
        super();
        this.state = {
            value: ".",
            rooms: [],
            selected: -1,
            stream: false,
            people: [],
            streams:[],
            nomistream : ["stream", "Background", "Raw foreground", "Filtered Foreground",
            "Disparity","Edge","Occupancy", "Height"],
        };

        this.handleSubmit = this.handleSubmit.bind(this);
        this.handleClick = this.handleClick.bind(this);
    }

    getRooms = () => {
        const self = this;
        fetch('http://localhost:8080/rooms')
            .then(response => response.json())
            .then(data => {
                // console.log(data);
                // var object = [];
                // data.forEach(function(value, key) {
                //     object.push(value);
                // });
                // console.log(object);
                self.state.rooms = data;
                self.forceUpdate();
            })
            .catch(err => {
                console.log("fetch error" + err);
            });

    }

    getStreams = () =>{
        var url = "http://localhost:8080/room/" + this.state.selected + "/streams";
        var self = this;
        fetch(url).then(response => response.json())
            .then(data => {
                console.log(data);
                var object = [];
                data.forEach(function(value, key) {
                    object.push("http://localhost:8080/stream/"+value);
                });
                self.state.streams = object;
                self.forceUpdate();
            })
            .catch(err => {
                console.log("fetch error" + err);
            });
    }

    getPeople = () => {
        var url = "http://localhost:8080/room/" + this.state.selected + "/people";
        var self = this;
        fetch(url)
            .then(response => response.json())
            .then(data => {
                self.state.people = data;
                self.forceUpdate();
                //self.getPeople();
            })
            .catch(err => {
                console.log("fetch error" + err);
            });
    };


    componentDidMount() {
        this.getRooms();
    }

    initializesys = (event) => {
        event.preventDefault();
        console.log(event.target);
        const data = new FormData(event.target);
        console.log(data);
        var url = "http://localhost:8080/room/" + this.state.selected + "/initialize_system";

        var object = {};
        data.forEach(function(value, key) {
            object[key] = value;
        });
        console.log(object);
        var json = JSON.stringify(object);
        console.log(json);
        var h = {
            method: 'POST',
            headers: {
                "username": "root",
                "password": "password",
                "type": "none",
                "resolution": "320x240",
                "fps": "10",
                "cameraModel": "Virtual Acquisition Camera",
                "IPAddress1": "192.168.8.3",
                "port1": "80",
                "IPAddress2": "192.168.8.2",
                "port2": "80"
            }
        };
        var h1 = {
            method: 'POST',
            headers: object
        };
        console.log(h);
        console.log(h1);
        fetch(url, h1).then(function(response) {
                console.log(response);
            })
            .then(responseData => {
                console.log(responseData);
                return responseData;
            })
            .then(data => {
                console.log(data);
            })
            .catch(err => {
                console.log("fetch error" + err);
            });


    }

    playerMode = (event) => {
        event.preventDefault();
        var url = "http://localhost:8080/room/" + this.state.selected + "/plathea_player";
        fetch(url)
        .then(function(response) {
            console.log(response);
        })
        .catch(err => {
            console.log("fetch error" + err);
        });
    }
    startLocalization = (event) => {
        event.preventDefault();
        var url = "http://localhost:8080/room/" + this.state.selected + "/start_localization_engine";
        fetch(url,
            {method: 'POST',
                "headers": {
                "withoutTracking": "0",
                "saveTracksToFile": "0",
                "cache-control": "no-cache",
                "Postman-Token": "24c8ccbd-fa6f-43b7-bd56-62417a3c3d2f"
                }
            })
        .then(function(response) {
            console.log(response);
        })
        .catch(err => {
            console.log("fetch error" + err);
        });
    }
    playerStart = (event) => {
        event.preventDefault();
        var self = this;
        var url = "http://localhost:8080/room/" + this.state.selected + "/plathea_player_start";
        fetch(url)
        .then(function(response) {
            console.log(response);
            self.getStreams();
            //self.getPeople();
        })
        .catch(err => {
            console.log("fetch error" + err);
        });
    }

    handleSubmit(event) {
        event.preventDefault();
        const self = this;
        const data = new FormData(event.target);
        var object = {};
        data.forEach(function(value, key) {
            object[key] = value;
        });
        var json = JSON.stringify(object);

        fetch('http://localhost:8080/room', {
                method: 'POST',
                headers: {
                    'Accept': 'application/json',
                    'Content-Type': 'application/json'
                },
                body: json,
            }).then(function(response) {
                self.setState({
                    value: "ok"
                });
                self.getRooms();
            })
            .then(responseData => {
                console.log(responseData);
                return responseData;
            })
            .then(data => {
                console.log(data);
            })
            .catch(err => {
                self.setState({
                    value: "error"
                });
                console.log("fetch error" + err);
            });
    }

    handleClick = roomID => {
        this.setState({
            selected: roomID
        });
    };



    render() {
        console.log("render");
        if (this.state.selected != -1)
            return ( <div id = "room" >
                <h1 > Room {
                    this.state.selected
                } </h1>
                load configuration file <FileUpload id = {
                    this.state.selected
                }
                call = {
                    "load_configuration_file"
                }/><br/ >
                internal calibration <FileUpload id = {
                    this.state.selected
                }
                call = {
                    "internal_calibration"
                }/><br/ >
                external calibration <FileUpload id = {
                    this.state.selected
                }
                call = {
                    "external_calibration"
                }/><br/ > <br/>
                svm classifier <FileUpload id = {
                    this.state.selected
                }
                call = {
                    "select_svm_classifier"
                }/><br/ > <br/>

                <form onSubmit = {
                    this.initializesys.bind(this)
                } >
                <label htmlFor = "username" > username </label> <input id = "username"
                name = "username"
                type = "text"/> <br/>
                <label htmlFor = "password" > password </label> <input id = "password"
                name = "password"
                type = "text"/> <br/>
                <label htmlFor = "type" > type </label> <input id = "type"
                name = "type"
                type = "text"/> <br/>
                <label htmlFor = "resolution" > resolution </label> <input id = "resolution"
                name = "resolution"
                type = "text"/> <br/>
                <label htmlFor = "fps" > fps </label> <input id = "fps"
                name = "fps"
                type = "text"/> <br/>
                <label htmlFor = "cameraModel" > cameraModel </label> <input id = "cameraModel"
                name = "cameraModel"
                type = "text"/> <br/>
                <label htmlFor = "IPAddress1" > IPAddress1 </label> <input id = "IPAddress1"
                name = "IPAddress1"
                type = "text"/> <br/>
                <label htmlFor = "port1" > port1 </label> <input id = "port1"
                name = "port1"
                type = "text"/> <br/>
                <label htmlFor = "IPAddress2" > IPAddress2 </label> <input id = "IPAddress2"
                name = "IPAddress2"
                type = "text"/> <br/>
                <label htmlFor = "port2" > port2 </label> <input id = "port2"
                name = "port2"
                type = "text"/> <br/>

                <button > initialize system </button> </
                form > <br/>
                <button onClick = {
                    this.startLocalization.bind(this)
                } > start Localization Engine </button><br/ >
                 <br/>
                <button onClick = {
                    this.playerMode.bind(this)
                } > player mode </button><br/ >
                <button onClick = {
                    this.playerStart.bind(this)
                } > player start </button><br/ >
                <br/>

                <button onClick = {
                    this.getPeople.bind(this)
                } > People </button><br/ >
                <ul>
                {
                    this.state.people.map((item, key) => {
                        console.log(item);
                        return <li key = {key}>
                            <Person id = {item.id} name={item.identity.firstname} x={item.position.x} y={item.position.y} /></li>
                    })
                }
                </ul>
                <ul>
                {
                    this.state.streams.map((item, key) => {
                        console.log(item);
                        return <li key = {key}>
                        <a style={{display: "table-cell"}} href={item} target="_blank">{this.state.nomistream[key]}</a>
                        </li>
                    })
                }
                </ul>
                </div>

            );
        return ( <div id = "rooms" >
            <form onSubmit = {
                this.handleSubmit
            } >

            <label htmlFor = "roomName" > Enter the name of the new room </label>
            <div class="room-wrapper"> <input id = "roomName" name = "roomName" type = "text"/>
            <button > New Room! </button> </div>
            </form > <div id = "response" > {
                this.state.value
            } </div> <div >
            <ul > {
                this.state.rooms.map((item, key) => {
                    console.log(item);
                    return <li key = {
                        key
                    } > <Item id = {
                        item.id
                    }
                    name = {
                        item.name
                    }
                    clickHandler = {
                        this.handleClick
                    }/></li >
                })
            } </ul> </div >

             </div>
        );
    }
}


export default App;
