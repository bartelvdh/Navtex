import * as React from 'react';


import { useState, useEffect } from 'react';
import './Navtex.css';

//const api_url_messages = "http://localhost:8080/config";
//const api_url_save_config = "http://localhost:8080/saveconfig";
const api_url_messages = "/config";
const api_url_save_config = "/saveconfig";

const stationletters = 'ABCDEFGHIJKLMNOPQRSTUVWX';
const messageletters = 'ABCDEFGHIJKLMNOPQRSTUVWXY';

const stationLetterarray = stationletters.split('');
const messageLetterarray = messageletters.split('');

const Stationarray = stationLetterarray.map(obj => ({
  letter: obj,
  active_flag: true
  
}))

const Messagearray = messageLetterarray.map(obj => ({
  letter: obj,
  active_flag: true,
  fixed_flag: (obj === "A"||obj==="B"||obj==="D"||obj==="L")? true:false
}))


const NavConfig = () => {



  const [Stations518, setStations518] = useState(Stationarray);
  const [Messages518, setMessages518] = useState(Messagearray);
  const [Stations490, setStations490] = useState(Stationarray);
  const [Messages490, setMessages490] = useState(Messagearray);


  useEffect(() => {
    //fetchData();
    console.log("Fetching config");
    fetchconfig();
  },[])


function updateStationarray (active_stations,freq) {

  var Stationarray = stationLetterarray.map(obj => ({
  letter: obj,
  active_flag: active_stations.includes(obj)
  }))
  switch(freq) {
    case 518:
      setStations518(Stationarray);     
      break;
    case 490:
      setStations490(Stationarray); 
      break;
    default:
      // code block
  }

}

function updateMessagesarray (active_messages,freq) {

  var Messagesarray = stationLetterarray.map(obj => ({
  letter: obj,
  active_flag: active_messages.includes(obj),
  fixed_flag: (obj === "A"||obj==="B"||obj==="D"||obj==="L")? true:false
  }))
  switch(freq) {
    case 518:
      setMessages518(Messagesarray);  
      break;
    case 490:
      setMessages490(Messagesarray);  
      break;
    default:
      // code block
  }
  
}

const fetchconfig =  () => {

    fetch(api_url_messages,
    {
     method : "GET",
     mode: 'cors',
     cache: "no-store"
    }
    )
        .then((res) => 
        {
             if (res.status >= 400) {
                 throw new Error("Server responds with error!");
             }
             console.log("get config");
             return res.json();
         })
         .then((d) => {
           console.log("got config");
           console.log(d);

           for (let i in d) {
            console.log("hci");
            console.log(i);
            switch(d[i].tag) {
              case "stations518":
                updateStationarray( d[i].value, 518 );
                break;
              case "messages518":
                updateMessagesarray( d[i].value, 518 )
                break;
              case "stations490":
                updateStationarray( d[i].value, 490 );
                break;
              case "messages490":
                updateMessagesarray( d[i].value, 490 )
                break;
              default:
                // code block
            }
           }

           
         })
         .catch((error) => {
           console.log('Request failed', error);
         }
         );
  }


  const saveconfig =  (what,letter_string) => {

    fetch(api_url_save_config+"/"+what+"/"+letter_string,
    {
     method : "GET",
     mode: 'cors',
     cache: "no-store"
    }
    )
        .then((res) => 
        {
             if (res.status >= 400) {
                 throw new Error("Server responds with error!");
             }
  
             console.log("taaaaat");
             //console.log(res);
             //console.log(res.json());
             return res.json();
         })
         .then((d) => {
           console.log("tuuuuuut");
           console.log(d);
         })
         .catch((error) => {
           console.log('Request failed', error);
         }
         );
  }



  function Station({ sname,status,ind,freq }) {

    function toggleStation(index) {
      let letter_string ="";
      let tmp_stations=[];
      switch(freq) {
       case 518:
         tmp_stations = [...Stations518]; 
         break;
       case 490:
         tmp_stations = [...Stations490]; 
         break;
       default:
         // code block
      }

      const nextStations = tmp_stations.map((c, i) => {
        let new_obj = {};
        if (i === index) {
          new_obj.letter = c.letter;
          new_obj.active_flag = !(c.active_flag);
        } else {
          new_obj.letter = c.letter;
          new_obj.active_flag = c.active_flag;
        };
        if (new_obj.active_flag) { 
          letter_string += c.letter; 
        }
        return new_obj;
      });
      console.log(letter_string);
      switch(freq) {
       case 518:
         setStations518(nextStations);
         saveconfig("stations518",letter_string);
         break;
       case 490:
         setStations490(nextStations);
         saveconfig("stations490",letter_string);
         break;
       default:
         // code block
      }
    }


    function handleStationClick(ind) {
      //  alert('You clicked me!  '+ ind );
      //  console.log(ind);
        toggleStation(ind) 
    }  
    
  
    if (status) {
      return (
        <li onClick={ () => {handleStationClick(ind)}}> {sname} </li> 
      )
    }
    else
    {
      return (
         <li onClick={ () => {handleStationClick(ind)}}>-</li>       
      )
    }
  
  }
    
  function Message({ sname,status,ind,freq }) {    

    function toggleMessage(index) {
      let letter_string ="";
      let tmp_messages = [];
      switch(freq) {
       case 518:
         tmp_messages = [...Messages518]; 
         break;
       case 490:
         tmp_messages = [...Messages490]; 
         break;
       default:
         // code block
      }
      const nextMessages = tmp_messages.map((c, i) => {
        let new_obj = {};
        if (!c.fixed_flag)
        {
         if (i === index) {
          new_obj.letter = c.letter;
          new_obj.active_flag = !(c.active_flag);
          new_obj.fixed_flag = c.fixed_flag;     
         } else {
          new_obj.letter = c.letter;
          new_obj.active_flag = c.active_flag;
          new_obj.fixed_flag = c.fixed_flag;
         }
  
        }
        else
        {
          new_obj.letter = c.letter;
          new_obj.active_flag = c.active_flag;
          new_obj.fixed_flag = c.fixed_flag;
        } 
        if (new_obj.active_flag) { 
          letter_string += c.letter; 
         }
        return new_obj;
      });
      console.log(letter_string);
      switch(freq) {
       case 518:
         setMessages518(nextMessages);
         saveconfig("messages518",letter_string);
         break;
       case 490:
         setMessages490(nextMessages);
         saveconfig("messages490",letter_string);
         break;
       default:
         // code block
      }
    }
  
    function handleMessageClick(ind) {
      //  alert('You clicked me!  '+ ind );
      //  console.log(ind);
        toggleMessage(ind) 
    }

    if (status) {
      return (
        <li onClick={ () => {handleMessageClick(ind)}}> {sname} </li> 
      )
    }
    else
    {
      return (
         <li onClick={ () => {handleMessageClick(ind)}}>-</li>       
      )
    }
  }

    return ( 
  

     <> 
    <div className="wrapper">
      <div>Stations 518kHz</div>
      <div className="stationselect">
       <ul>
         { Stations518.map((a,i) => (
           <Station key={a.letter} sname={a.letter} status={a.active_flag} ind={i} freq={518}></Station>
         ))}
       </ul>
     </div>
     <br></br>
     <div>Message types 518kHz</div>
     <div className="messageselect">
       <ul>
         { Messages518.map((b,i) => (
           <Message key={b.letter} sname={b.letter} status={b.active_flag} ind={i} freq={518}></Message>
         ))}
       </ul>
     </div>
    </div>

    <div className="wrapper">
     <div>Stations 490kHz</div>
      <div className="stationselect">
       <ul>
         { Stations490.map((a,i) => (
           <Station key={a.letter} sname={a.letter} status={a.active_flag} ind={i} freq={490}></Station>
         ))}
       </ul>
     </div>
     <br></br>
     <div>Message types 490kHz</div>
     <div className="messageselect">
       <ul>
         { Messages490.map((b,i) => (
           <Message key={b.letter} sname={b.letter} status={b.active_flag} ind={i} freq={490}></Message>
         ))}
       </ul>
     </div>
    </div> 
    </>
    );

}


export default NavConfig;