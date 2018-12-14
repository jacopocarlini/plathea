import React from "react";
import PropTypes from "prop-types";
import Room from "./Room";


class Person extends React.Component {
    render() {
        return (
            <div >
                Person {this.props.id} : ({this.props.x}, {this.props.y}) {this.props.name}
            </div>
        );
    }
}

Person.propTypes = {
    id: PropTypes.number,
    name: PropTypes.string,
    x: PropTypes.number,
    y: PropTypes.number,
};

export default Person;
