import {defineStore} from 'pinia'
import {ref} from 'vue'


export const useAppStore =  defineStore('app', ()=>{

    /*  
    The composition API way of defining a Pinia store
    ref() s become state properties
    computed() s become getters
    function() s become actions  
    */ 

    // STATES 
  


    // ACTIONS
      const getUpdateData= async (data) => {
          const controller = new AbortController();
          const signal = controller.signal;
          const id= setTimeout(() => controller.abort(), 60000);
          const URL= '/api/update';
          try {
                  const response = await fetch(URL, {method: 'POST', signal: signal, headers: {'Content-Type': 'application/json'}, body: JSON.stringify(data)});
                  if (response.ok) {
                    const result = await response.json();
                    clearTimeout(id);
                    return result;
                  }
              } catch (e) {
                  clearTimeout(id);
                  console.error("getUpdateData error:", e.message);
                  return {status: 'getUpdateData error', message: e.message};
              }
        }
  
        const updateCard = async (start,end) => {
            const controller = new AbortController();
            const signal = controller.signal;
            const id = setTimeout(() => { controller.abort() }, 60000);
            const URL = `/api/avg/${start}/${end}`;
            try {
                // console.log(`updateCard data for ${start} to ${end}`);
                const response = await fetch(URL, { method: 'GET', signal: signal });
                if (response.ok) {
                  const data = await response.json();
                  // console.log(data);
                  if (data && data.status === "found" && data.data) {
                      return data.data;
                  }
                }
            }
            catch (e) {
                console.error('updateCard error:', e.message);
            }
            return [];
        }

        const setPasscode = async (passcode) => {
            const controller = new AbortController();
            const signal = controller.signal;
            const id = setTimeout(() => { controller.abort() }, 60000);
            const URL = `/api/set/combination`;
            console.log(passcode);
          
            let text = passcode.toString();
          
            try {
                const response = await fetch(URL, { method: 'POST', signal: signal, body: JSON.stringify({"code": text}), headers: {'Content-Type': 'application/json'}});
                if (response.ok) {
                  const result = await response.json();
                  clearTimeout(id);
                  return result;
                }
            }
            catch (e) {
                clearTimeout(id);
                console.error("setPasscode error:", e.message);
                return {status: 'setPasscode error', message: e.message};
            }
        }

        const checkPasscode= async (passcode) => {
            const controller = new AbortController();
            const signal = controller.signal;
            const id= setTimeout(() => controller.abort(), 60000);
            const URL= '/api/check/combination'; 
            try {
                const response = await fetch(URL, {method: 'POST', signal: signal, headers: {'Content-Type': 'application/json'}, body: JSON.stringify(passcode)});
                if (response.ok) {
                  const result = await response.json();
                  // console.log(data);
                  clearTimeout(id);
                  return result;
                }
            } catch (e) {
                clearTimeout(id);
                console.error("checkPasscode error:", e.message);
                return {status: 'checkPasscode error', message: e.message};
            }
        }
                
        const getReserve = async (start, end) => {
            const controller = new AbortController();
            const signal = controller.signal;
            const id = setTimeout(() => { controller.abort() }, 60000);
            const URL = `/api/reserve/${start}/${end}`;
            try {
                console.log(`getReserve data for ${start} to ${end}`);
                const response = await fetch(URL, { method: 'GET', signal: signal });
                if (response.ok) {
                  const data = await response.json();
                  // console.log(data);
                  if (data && data.status === "found" && data.data) {
                      return data.data;
                  }
                }
            }
            catch (e) {
                console.error('getReserve error:', e.message);
            }
            return [];
        }
  
 
    return { 
    // EXPORTS	
        getUpdateData,
        setPasscode,
        getCheckPwd,
        updateCard,
        getReserve
    }
},{ persist: true  });
 
