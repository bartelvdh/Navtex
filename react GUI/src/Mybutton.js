import React from 'react';

// #region constants

// #endregion

// #region styled-components

// #endregion

// #region functions

// #endregion

// #region component
const propTypes = {};

const defaultProps = {};

/**
 * 
 */

fetch("https://csme.be/eventc.php/")
  .then((response) => response.json())
  .then((data) => 
    {
    console.log(data);
    const myelem=data.features[0].properties.name;
    console.log(myelem);
    } 
  
  );




const products = [
    { title: 'Cabbage', id: 1 },
    { title: 'Garlic', id: 2 },
    { title: 'Apple', id: 3 },
  ];

const listItems = products.map(product =>
    <li key={product.id}>
      {product.title}
    </li>
  );


const Mybutton = ({ evsols }) => {
    return ( 
     <div>
        <ul>{listItems}</ul>
        <button>Hello World</button>
    </div>
    );
}

Mybutton.propTypes = propTypes;
Mybutton.defaultProps = defaultProps;
// #endregion

export default Mybutton;