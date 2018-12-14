import React from "react";
import PropTypes from "prop-types";

class Room extends React.Component {

    constructor(props) {
        super(props);
        this.state = {
            people: [],
        }
    }

    getPeople = () => {
        var url = "http://localhost:8080/room/" + this.props.id + "/people";
        var self = this;
        fetch(url).then(function(response) {
                console.log(response);
                self.setState({people:response});
            })
            .catch(err => {
                console.log("fetch error" + err);
            });
    };

  render() {
    return (
      <div>
      {this.state.people}
      </div>
    );
  }
}

Room.propTypes = {
  id: PropTypes.number
};

export default Room;
