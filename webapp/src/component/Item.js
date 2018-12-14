import React from "react";
import PropTypes from "prop-types";
import Room from "./Room";


class Item extends React.Component {
    handleClick = () => {
        this.props.clickHandler(this.props.id);
    };

    render() {
        return ( <
            div >
            <
            button onClick = {
                this.handleClick
            } > Room {
                this.props.id
            }: {
                this.props.name
            } < /button> <
            /div>
        );
    }
}

Item.propTypes = {
    id: PropTypes.number,
    name: PropTypes.string,
    clickHandler: PropTypes.func,
};

export default Item;
