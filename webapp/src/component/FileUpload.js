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
        this.setState({
            selectedFiles: event.target.files,
        })
    }


    render() {
        return ( <
            div className = "container" >
            <
            input type = "file"
            name = ""
            id = ""
            onChange = {
                this.handleselectedFile
            }
            required multiple / >
            <
            button onClick = {
                this.handleUpload
            } > Upload < /button> <
            /div>
        )
    }

}


FileUpload.propTypes = {
    id: PropTypes.number,
    call: PropTypes.string,
};

export default FileUpload;
