import React, {
    Component
} from 'react';
import axios from 'axios';
import PropTypes from "prop-types";


class FileUpload extends Component {

    constructor(props) {
        super(props);
        this.state = {
            selectedFiles: null,
            filestring: "Choose the files",
        }


    }


    handleUpload = () => {
        const data = new FormData()
        console.log(this.state.selectedFiles);
        var file = 'file';
        if (this.state.selectedFiles.length > 1) file = 'files';
        console.log(this.state.selectedFiles.length);
        console.log(file);
        for (var i = 0; i < this.state.selectedFiles.length; i++) {
            console.log(i);
            data.append(file, this.state.selectedFiles[i], this.state.selectedFiles[i].name)

        }
        console.log(data);
        var roomID = "" + this.props.id;
        var url = "http://localhost:8080/room/" + roomID + "/" + this.props.call;
        console.log(url);

        axios
            .post(url, data, {
                headers: {
                    "mask": 7
                }
            }, )
            .then(res => {
                console.log(res.statusText)
            })

    }

    handleselectedFile = event => {
        console.log(event.target.files);
        this.setState({
            filestring: "Loaded"
        });
        this.setState({
            selectedFiles: event.target.files,
        })
        console.log("fatto");
    }


    render() {
        return (
            <div class = "custom-file-upload" >
                <input type = "file" class="inputfile" name="" id = {this.props.call}
                    onChange = {this.handleselectedFile} required multiple/>
                    <label htmlFor={this.props.call} required multiple>{this.state.filestring}</label>
                <button class="upload" onClick = {this.handleUpload}>Upload</button>
            </div>

        )
    }

}


FileUpload.propTypes = {
    id: PropTypes.number,
    call: PropTypes.string,
};

export default FileUpload;
