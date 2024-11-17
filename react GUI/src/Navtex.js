import * as React from 'react';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import Box from '@mui/material/Box';

import { useState, useEffect } from 'react';
import './Navtex.css';
import NavConfig from "./NavConfig";


//const api_url_messages="http://localhost:8080/messagelist";
//const api_url_messagetext="http://localhost:8080/messagetext";

const api_url_messages="/messagelist";
const api_url_messagetext="/messagetext";

const Navtex = () => {




  const [value, setValue] = React.useState(0);
  const [messages,setMessages] = useState( () =>{ return [] });
  const [messagetoshow,setMessagetoshow] = useState( () =>{ return 0 });
  const [messagetoshowtext,setMessagetoshowtext] = useState( () =>{ return " " });

  const handleChange = (event: React.SyntheticEvent, newValue: number) => {
    setValue(newValue);
  };


  useEffect(() => {
    fetchmessages();
    setInterval(fetchmessages, 121000);
  },[])


const fetchmessages =  () => {

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
           //console.log(res);
           //console.log(res.json());
           return res.json();
       })
       .then((d) => {
         console.log(d);
         setMessages(d);
       })
       .catch((error) => {
         console.log('Request failed', error);
       }
       );
}




const fetchmessagetext =  (m_id) => {

  fetch(api_url_messagetext+"/"+m_id.toString(),
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
         console.log(d.message);
         setMessagetoshow(m_id);
         setMessagetoshowtext(d.message.replace(/\n/g, "<br/>"))
       })
       .catch((error) => {
         console.log('Request failed', error);
       }
       );
}

  interface TabPanelProps {
    children?: React.ReactNode;
    index: number;
    value: number;
  }
  
  function CustomTabPanel(props: TabPanelProps) {
    const { children, value, index, ...other } = props;
  
    return (
      <div
        role="tabpanel"
        hidden={value !== index}
        id={`simple-tabpanel-${index}`}
        aria-labelledby={`simple-tab-${index}`}
        {...other}
      >
        {value === index && <Box sx={{ p: 3 }}>{children}</Box>}
      </div>
    );
  }
  
  function a11yProps(index: number) {
    return {
      id: `simple-tab-${index}`,
      'aria-controls': `simple-tabpanel-${index}`,
    };
  }
  
  function showmessage(id:number) {
    console.log(id);
    
    fetchmessagetext(id);
  }

  function hidemessage() {

    setMessagetoshow(0);
    setMessagetoshowtext(" ");
    fetchmessages();
  }

    return ( 
    <>


    <Box sx={{ width: '100%' }}>
      <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
        <Tabs value={value} onChange={handleChange} aria-label="basic tabs example">
          <Tab label="Messages" {...a11yProps(0)} />
          <Tab label="Configuration" {...a11yProps(1)} />
        </Tabs>
      </Box>

      <CustomTabPanel value={value} index={0}>
    {messagetoshow === 0
      ? 
       <div className="messagelist">
        <table>
        <tbody> 
        {messages.map((m_object,m_index) =>
          <tr key={m_index} className={ m_object.age==="NEW"  ? 'newmessage': 'oldmessage' }>
            <td key="00" onClick={ () => {showmessage(m_object.id)}}> {m_object.bbbb} </td>
            <td key="01" onClick={ () => {showmessage(m_object.id)}}> {m_object.type} </td>
            <td key="02" onClick={ () => {showmessage(m_object.id)}}> {m_object.timestamp} </td>
            <td key="03"> {m_object.age} </td>
         </tr>
         )
        }
        </tbody>
        </table>
       </div>
      : 
        <div className="navtextbox" onClick={ () => {hidemessage()}}>
          <p className="mypar" dangerouslySetInnerHTML={{__html: messagetoshowtext}} />
        </div>
    }
      </CustomTabPanel>
      <CustomTabPanel value={value} index={1}>

         <NavConfig />

      </CustomTabPanel>
  
    </Box>




    </>
    );

}


export default Navtex;