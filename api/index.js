import * as jose from 'jose';
import * as util from './util.js';
import * as allowList from './allow-list.js'
import { Toucan } from 'toucan-js';

addEventListener('fetch', event => {
  if (MAINTENANCE_SWITCH) {
    event.respondWith(switchToMaintenance());
  } else {
    event.respondWith(handleRequest(new Toucan({
      dsn: SENTRY_DSN,
      context: event,
      request: event.request,
      requestDataOptions: {
        allowedHeaders: true,
        allowedCookies: true,
        allowedSearchParams: true,
        allowedIps: true
      }
    }), event.request));
  }
})

async function switchToMaintenance() {
  return await util.unsuccessfulResponse('We are currently under maintenance. Please try again later.');
}

async function handleRequest(sentry, request) {
  if (request.method !== 'POST') {
    return new Response(null, { status: 403 });
  }

  try {
    const { headers } = request;
    const clientIP = headers.get('CF-Connecting-IP');
    const clientRay = headers.get('CF-Ray');

    if (!clientIP) {
      util.sentryMessage(`No client ip: ${JSON.String(...headers)}`);
      return new Response(null, { status: 403 });
    }

    if (!clientRay) {
      util.sentryMessage(`No client ray: ${JSON.String(...headers)}`);
      return new Response(null, { status: 403 });
    }

    const formData = await request.formData();
    const body = {};

    for (const entry of formData.entries()) {
      body[entry[0]] = await util.decryptRequest(entry[1]);
    }

    sentry.setRequestBody(body);

    if (!body['endpoint']) {
      throw 'No endpoint was provided';
    }

    if (body['endpoint'] === 'fetch') {
      return await handleFetch(sentry, clientRay, body['payload']);
    }
    else if (body['endpoint'] === 'auth') {
      return await handleAuthentication(sentry, clientRay, body['payload'], body['endpoint_key'], clientIP);
    }
    else if (body['endpoint'] === 'auth_tfa') {
      return await handleStream(sentry, clientRay, body['payload'], body['endpoint_key']);
    }
    else if (body['endpoint'] === 'streamv2') {
      return await handleStreamV2(sentry, clientRay, body['endpoint_key']);
    }
    else {
      throw `Unsupported endpoint ${body['endpoint']} | ID: ${clientRay}`;
    }
  }
  catch (e) {
    util.sentryError(e.message, sentry);
    return await util.unsuccessfulResponse('Something went wrong');
  }
}

async function handleFetch(sentry, clientRay, payload) {
  try {
    if (!payload || payload === 'null') {
      util.sentryMessage(`0x00001000: payload is empty`, sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00001000) | ID: ${clientRay}`);
    }

    payload = JSON.parse(payload);

    if (!payload.client_version || payload.client_version !== CLIENT_VERSION) {
      return await util.unsuccessfulResponse('Client is outdated. Please download the new version at https://STRIPPED_FOR_PUBLIC_RELEASE.com/panel/');
    }

    const endpointKey = await new jose.SignJWT({
      'endpoint': 'auth',
      'client_version': payload.client_version,
      'id': clientRay
    })
    .setProtectedHeader({ alg: 'HS256' })
    .setIssuedAt()
    .setNotBefore(Math.floor(Date.now() / 1000))
    .setExpirationTime(Math.floor(Date.now() / 1000) + 360) // 6 min session
    .setIssuer('quicksolve')
    .sign(new TextEncoder().encode(JWT_SERVER_KEY));

    return await util.successfulResponse('auth', endpointKey, 'login');
  }
  catch (e) {
    util.sentryError(e.message, sentry);
    return await util.unsuccessfulResponse(`Something went wrong (0x00001100) | ID: ${clientRay}`);
  }
}

async function handleAuthentication(sentry, clientRay, payload, payloadEndpointKey, clientIP) {
  try {
    if (!payloadEndpointKey || payloadEndpointKey === 'null') {
      util.sentryMessage(`0x00002000: payloadEndpointKey is empty`, sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00002000) | ID: ${clientRay}`);
    }

    try {
      var decodedEndpointKey = await jose.jwtVerify(payloadEndpointKey, new TextEncoder().encode(JWT_SERVER_KEY), {
        maxTokenAge: '6 minutes'
      });
    } catch (e) {
      return await util.unsuccessfulResponse(`Your session has expired (0x00002001) | ID: ${clientRay}`);
    }

    if (decodedEndpointKey.payload.client_version !== CLIENT_VERSION || decodedEndpointKey.payload.endpoint !== 'auth') {
      util.sentryMessage(`0x00002002: Failed check: ${decodedEndpointKey.client_version}, ${decodedEndpointKey.endpoint}`, sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00002002) | ID: ${clientRay}`);
    }

    if (!payload || payload === 'null') {
      util.sentryMessage('0x00002003: Payload empty', sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00002003) | ID: ${clientRay}`);
    }

    payload = JSON.parse(payload);

    if (!payload.username || !payload.password) {
      util.sentryMessage(`0x0000200X: One or more empty: ${JSON.stringify(payload)}`, sentry);
      return await util.unsuccessfulResponse(`Username and or password is empty | ID: ${clientRay}`);
    }

    if (payload.fp.length === 0) {
      util.sentryMessage(`0x0000201X: Empty fingerprints: ${JSON.stringify(payload)}`, sentry);
      return await util.unsuccessfulResponse(`Something went wrong (0x0000201X) | ID: ${clientRay}`);
    }

    // filter the valid fingerprints
    let validFingerprints = [];
    await Promise.all(payload.fp.fingerprints.map(async (element) => {
      if (await allowList.isFingerprintAllowed(element.m_subkey, element.m_key)) {
        validFingerprints.push(element);
      }
    }));

    let clientSTRIPPED_FOR_PUBLIC_RELEASEData = [];
    await Promise.all(payload.fp.STRIPPED_FOR_PUBLIC_RELEASE_data.map(async (element) => {
      // make sure our data is sanitized
      if (!isNaN(parseInt(element))) {
        clientSTRIPPED_FOR_PUBLIC_RELEASEData.push(element);
      }
    }));

    if (clientSTRIPPED_FOR_PUBLIC_RELEASEData.length === 0)
      clientSTRIPPED_FOR_PUBLIC_RELEASEData.push(0);

    // Encapsulate our POST data into URLSearchParams so our parameters cannot be manipulated
    // e.g: client sends us 'account&username=account2' which would result in username=account&username=account2 in our POST body
    const apiRequest = {
      method: 'POST',
      headers: {
        'STRIPPED_FOR_PUBLIC_RELEASE-api-key': STRIPPED_FOR_PUBLIC_RELEASE_API_KEY,
        'content-type': 'application/x-www-form-urlencoded',
        'user-agent': 'STRIPPED_FOR_PUBLIC_RELEASE_CF_WORKER'
      },
      body: new URLSearchParams({
        'login': `${payload.username}`,
        'password': `${payload.password}`,
        'limit_ip': `${clientIP}`
      })
    };

    const apiResponse = await fetch('https://STRIPPED_FOR_PUBLIC_RELEASE.com/api/auth', apiRequest);

    if (apiResponse.status >= 500) {
      return await util.unsuccessfulResponse('Server is temporarily offline or unavailable. Please try again later');
    }

    const responseJSON = await apiResponse.json();

    if (apiResponse.status != 200) {
      return await util.unsuccessfulResponse(responseJSON.errors[0].message);
    }

    if (responseJSON.success === false) {
      return await util.unsuccessfulResponse('Account is not in a valid state');
    }

    // Ensure the user is in a valid state and has an active subscription
    if ((responseJSON.user.is_banned !== null && responseJSON.user.is_banned === true)
    || responseJSON.user.user_state !== "valid"
    || (!responseJSON.user.secondary_group_ids.includes(6)
	  && !responseJSON.user.secondary_group_ids.includes(7)
    && !responseJSON.user.secondary_group_ids.includes(8))) {
      return await util.unsuccessfulResponse('Your account does not have an active subscription. Please get a subscription at https://STRIPPED_FOR_PUBLIC_RELEASE.com/account/upgrades. If you already have a subscription, you may need to wait 5 minutes for your subscription to fully activate');
    }

    // form a base for making requests to our hwid endpoints
    /*
      (log) {
      m_high_date: 30892655,
      m_key: 'a4fe65264ef7dbb38d104b1e81eb3350f3142f3d16f32bdec39b1d9b42c1b8d1',
      m_low_date: 2976812010,
      m_quadpart: 132682945888422900,
      m_subkey: '460b1289d0d35b9726a08cd08ea8b78563b6e1b9e51ed4db050f61aa92f9fd1a',
      m_timestamp: 1623820988,
      m_value: 'b6234d2ea0d6022be63db80d7b80e221097fe4a469dc44febcd2a9241effdeba'
    }
    */
    if (INTERNAL_TESTS_HWID) {
      let hwidRequest = {
        method: 'POST',
        headers: {
          'STRIPPED_FOR_PUBLIC_RELEASE-api-key': STRIPPED_FOR_PUBLIC_RELEASE_API_KEY,
          'content-type': 'application/x-www-form-urlencoded',
          'user-agent': 'STRIPPED_FOR_PUBLIC_RELEASE_CF_WORKER'
        },
        body: new URLSearchParams({
          'user_id': `${responseJSON.user.user_id}`
        })
      };

      const hwidResponse = await fetch('https://STRIPPED_FOR_PUBLIC_RELEASE.com/api/hwid/get', hwidRequest);

      if (hwidResponse.status !== 200) {
        util.sentryMessage(`0x00002004: hwidResponse returned status ${hwidResponse.status} [[DUMP]] ${await hwidResponse.text()}`, sentry, util.SENTRY_LEVELS.ERROR);
        return await util.unsuccessfulResponse(`Something went wrong (0x00002004) | ID: ${clientRay}`);
      }

      const hwidJSON = await hwidResponse.json();

      if (hwidJSON.success === false) {
        hwidRequest.body = new URLSearchParams({
          'user_id': `${responseJSON.user.user_id}`,
          'hwid': `${JSON.stringify(validFingerprints)}`,
          'STRIPPED_FOR_PUBLIC_RELEASE_data': `${JSON.stringify(clientSTRIPPED_FOR_PUBLIC_RELEASEData)}`,
          'ip': `${clientIP}`
        });

        const setHWIDResponse = await fetch('https://STRIPPED_FOR_PUBLIC_RELEASE.com/api/hwid/set', hwidRequest);

        if (setHWIDResponse.status !== 200) {
          util.sentryMessage(`0x00002005: hwidResponse returned status ${setHWIDResponse.status} [[DUMP]] ${await setHWIDResponse.text()}`, sentry, util.SENTRY_LEVELS.ERROR);
          return await util.unsuccessfulResponse(`Something went wrong (0x00002005) | ID: ${clientRay}`);
        }

        const hwidResponseJSON = await setHWIDResponse.json();

        if (hwidResponseJSON.success === false) {
          util.sentryMessage(`0x00002006: hwidResponse returned status ${hwidResponseJSON.status} [[DUMP]] ${JSON.stringify(hwidResponseJSON)}`, sentry);
          return await util.unsuccessfulResponse(`Something went wrong (0x00002006) | ID: ${clientRay}`);
        }
      } else if ((responseJSON.user.secondary_group_ids.includes(7) || responseJSON.user.secondary_group_ids.includes(8)) && hwidJSON.success !== false) {
        // only enforce hwid check if in paid user group and successful.
        // compare the HWID from the database to the HWID we're presented with
        console.log('--SUCCESS TRUE--');

        let hwids = JSON.parse(hwidJSON.response.hwid);
        let hwidHasMismatch = false;

        for (const e of hwids) {
          if (hwidHasMismatch) {
            break;
          }

          let foundMismatch = true;

          for (const vf of validFingerprints) {
            if (!foundMismatch) {
              break;
            }

            if (vf.m_key === e.m_key && vf.m_subkey === e.m_subkey) {
              // skip info key
              if (vf.m_key === await util.sha256hash('STRIPPED_FOR_PUBLIC_RELEASE') || vf.m_key === await util.sha256hash('STRIPPED_FOR_PUBLIC_RELEASE')) {
                foundMismatch = false;
              } else if (vf.m_low_date === e.m_low_date && vf.m_high_date === e.m_high_date && vf.m_quadpart === e.m_quadpart && vf.m_timestamp === e.m_timestamp) {
                foundMismatch = false;
              }
            }
          }

          if (foundMismatch) {
            console.log(`Mismatch: ${JSON.stringify(e)}`);
            hwidHasMismatch = true;
          }
        }

        if (hwidHasMismatch) {
          // send all the data to the server
          // include the STRIPPED_FOR_PUBLIC_RELEASE data collected
          hwidRequest.body = new URLSearchParams({
            'user_id': `${responseJSON.user.user_id}`,
            'hwid': `${JSON.stringify(validFingerprints)}`,
            'STRIPPED_FOR_PUBLIC_RELEASE_data': `${JSON.stringify(clientSTRIPPED_FOR_PUBLIC_RELEASEData)}`,
            'ip': `${clientIP}`
          });
  
          const setMismatchHwidResponse = await fetch('https://STRIPPED_FOR_PUBLIC_RELEASE.com/api/hwid/set_mismatch', hwidRequest);

          let text = await setMismatchHwidResponse.text();
          console.log(text);

          if (setMismatchHwidResponse.status !== 200) {
            util.sentryMessage(`0x00002007: setMismatchHwidResponse returned status ${setMismatchHwidResponse.status} [[DUMP]] ${text}`, sentry, util.SENTRY_LEVELS.ERROR);
            return await util.unsuccessfulResponse(`Something went wrong (0x00002007) | ID: ${clientRay}`);
          }

          // return error saying hwid mismatch
          return await util.unsuccessfulResponse('Login failed. A hardware reset ticket has been automatically created, please see https://STRIPPED_FOR_PUBLIC_RELEASE.com/panel/ for more information');
        } else {
          // there was no mismatch, but lets check if theres new data the loader collected
          // clone our array
          let newHwids = [...hwids];
          let shouldSendRequest = false;

          for (const vf of validFingerprints) {
            let missing = true;

            for (const hwid of hwids) {
              if (vf.m_key === hwid.m_key && vf.m_subkey === hwid.m_subkey) {
                missing = false;
                break;
              }
            }

            if (missing) {
              newHwids.push(vf);
              console.log('NEW HWID:' + JSON.stringify(vf));
              shouldSendRequest = true;
            }
          }

          // given our STRIPPED_FOR_PUBLIC_RELEASE data from database, compare to what client gives and append to STRIPPED_FOR_PUBLIC_RELEASEData if we have more information, and send to database.
          let svSTRIPPED_FOR_PUBLIC_RELEASEData = JSON.parse(hwidJSON.response.STRIPPED_FOR_PUBLIC_RELEASE_data);
          let newSTRIPPED_FOR_PUBLIC_RELEASEData = [...svSTRIPPED_FOR_PUBLIC_RELEASEData];

          // loop through what the client presents us with
          for (const clSTRIPPED_FOR_PUBLIC_RELEASEDat of clientSTRIPPED_FOR_PUBLIC_RELEASEData) {
            // check if server STRIPPED_FOR_PUBLIC_RELEASEdata already has this
            if (svSTRIPPED_FOR_PUBLIC_RELEASEData.includes(clSTRIPPED_FOR_PUBLIC_RELEASEDat)) {
              continue;
            }

            // otherwise, add to our newSTRIPPED_FOR_PUBLIC_RELEASEData array
            newSTRIPPED_FOR_PUBLIC_RELEASEData.push(clSTRIPPED_FOR_PUBLIC_RELEASEDat);
            shouldSendRequest = true;
          }

          // same logic as setting hwid as if its a clean state
          /*if (shouldSendRequest) {
            hwidRequest.body = new URLSearchParams({
              'user_id': `${responseJSON.user.user_id}`,
              'hwid': `${JSON.stringify(newHwids)}`,
              'STRIPPED_FOR_PUBLIC_RELEASE_data': `${JSON.stringify(newSTRIPPED_FOR_PUBLIC_RELEASEData)}`,
              'ip': `${clientIP}`
            });

            const updateHWIDResponse = await fetch('https://STRIPPED_FOR_PUBLIC_RELEASE.com/api/hwid/update', hwidRequest);

            if (updateHWIDResponse.status !== 200) {
              util.sentryMessage(`0x00002008: updateHWIDResponse returned status ${updateHWIDResponse.status}`, sentry, util.SENTRY_LEVELS.ERROR);
              return await util.unsuccessfulResponse(`Something went wrong (0x00002008) | ID: ${clientRay}`);
            }

            const updateHWIDResponseJSON = await updateHWIDResponse.json();

            if (updateHWIDResponseJSON.success === false) {
              util.sentryMessage(`0x00002009: updateHWIDResponseJSON returned false [[DUMP]] ${JSON.stringify(updateHWIDResponseJSON)}`, sentry);
              return await util.unsuccessfulResponse(`Something went wrong (0x00002009) | ID: ${clientRay}`);
            }
          }*/
        }
      }
    }

    // we're going to sign a JWT instead for the client to pass back to us
    const endpointKey = await new jose.SignJWT({
      'endpoint': 'auth_tfa',
      'username': responseJSON.user.username,
      'packages': responseJSON.user.secondary_group_ids,
      'STRIPPED_FOR_PUBLIC_RELEASE_data': clientSTRIPPED_FOR_PUBLIC_RELEASEData,
      'id': clientRay
    })
    .setProtectedHeader({ alg: 'HS256' })
    .setIssuedAt()
    .setNotBefore(Math.floor(Date.now() / 1000))
    .setExpirationTime(Math.floor(Date.now() / 1000) + 360)
    .setIssuer('quicksolve')
    .sign(new TextEncoder().encode(JWT_SERVER_KEY));

    let levelOfAccess = '0';

    // beta
    if (responseJSON.user.secondary_group_ids.includes(8)) {
      levelOfAccess = '3';
    }
    // standard
    else if (responseJSON.user.secondary_group_ids.includes(7)) {
      levelOfAccess = '2';
    }
    // lite
    else if (responseJSON.user.secondary_group_ids.includes(6)) {
      levelOfAccess = '1';
    }

    return await util.successfulResponse('auth_tfa', endpointKey, 'stream', levelOfAccess);
  }
  catch (e) {
    util.sentryError(e.message, sentry);
    return await util.unsuccessfulResponse(`Something went wrong (0x00002100) | ID: ${clientRay}`);
  }
}

async function handleStream(sentry, clientRay, payload, payloadEndpointKey) {
  try {
    if (!payloadEndpointKey || payloadEndpointKey === 'null') {
      util.sentryMessage('0x00003000: payloadEndpointKey is empty', sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00003000) | ID: ${clientRay}`);
    }

    try {
      var decodedEndpointKey = await jose.jwtVerify(payloadEndpointKey, new TextEncoder().encode(JWT_SERVER_KEY), {
        maxTokenAge: '6 minutes'
      });
    } catch (e) {
      return await util.unsuccessfulResponse(`Your session has expired (0x00003001) | ID: ${clientRay}`);
    }

    if (decodedEndpointKey.payload.endpoint !== 'auth_tfa' || !decodedEndpointKey.payload.username || decodedEndpointKey.payload.username === 'null') {
      util.sentryMessage(`0x00003002: Failed check: ${decodedEndpointKey.payload.endpoint}, ${decodedEndpointKey.payload.username}`, sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00003002) | ID: ${clientRay}`);
    }

    if (!payload || payload === 'null') {
      util.sentryMessage('0x00003003: Payload is empty', sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00003003) | ID: ${clientRay}`);
    }

    payload = JSON.parse(payload);

    if (!payload.branch) {
      util.sentryMessage('0x00003004: payload.branch is empty', sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00003004) | ID: ${clientRay}`);
    }

    // hash all the STRIPPED_FOR_PUBLIC_RELEASE ids
    let hashedClSTRIPPED_FOR_PUBLIC_RELEASEData = [];
    for (const clSTRIPPED_FOR_PUBLIC_RELEASEDat of decodedEndpointKey.payload.STRIPPED_FOR_PUBLIC_RELEASE_data) {
      if (typeof clSTRIPPED_FOR_PUBLIC_RELEASEDat !== 'string') {
        break;
      }

      hashedClSTRIPPED_FOR_PUBLIC_RELEASEData.push(await util.sha256hash(clSTRIPPED_FOR_PUBLIC_RELEASEDat));
    }

    console.log(hashedClSTRIPPED_FOR_PUBLIC_RELEASEData.toString());

    // We're just going to pass offsets and keys, we dont need another request
    var offsets = {
      'success': true,
      'response': {
        'username': decodedEndpointKey.payload.username,
        'branch': payload.branch,
        'timestamp': Math.floor(Date.now() / 100000),
        'hashes': hashedClSTRIPPED_FOR_PUBLIC_RELEASEData.toString(),
        'modules': {},
        'signatures': {}
      }
    };

    offsets = await util.createOffsets(offsets, payload.branch === 'lite');

    // we're going to pass the pipe name and pipe key via the endpoint, as a nested JSON string
    var pipe = {
      'name': `${crypto.randomUUID()}`,
      'key': `${String(await util.mutateResponse(crypto.randomUUID())).substring(0, 32)}`,
      'data': `${JSON.stringify(offsets)}`
    };

    // we need this for our stream endpoint
    const endpointKey = await new jose.SignJWT({
      'endpoint': 'fetchv2',
      'username': decodedEndpointKey.payload.username,
      'branch': payload.branch,
      'packages': decodedEndpointKey.payload.packages,
      'id': clientRay
    })
    .setProtectedHeader({ alg: 'HS256' })
    .setIssuedAt()
    .setNotBefore(Math.floor(Date.now() / 1000))
    .setExpirationTime(Math.floor(Date.now() / 1000) + 10)
    .setIssuer('quicksolve')
    .sign(new TextEncoder().encode(JWT_SERVER_KEY));

    return await util.successfulResponse(JSON.stringify(pipe), endpointKey, 'load', '0', 'streamv2');
  }
  catch (e) {
    util.sentryError(`${e.message}`, sentry);
    return await util.unsuccessfulResponse(`Something went wrong (0x00003100) | ID: ${clientRay}`);
  }
}

// we cant put our stream into a json key so we have to create a new endpoint that will allow the client to request the module from us
async function handleStreamV2(sentry, clientRay, payloadEndpointKey) {
  try {
    if (!payloadEndpointKey || payloadEndpointKey === 'null') {
      util.sentryMessage('0x00004000: payloadEndpointKey is empty', sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00004000) | ID: ${clientRay}`);
    }

    try {
      var decodedEndpointKey = await jose.jwtVerify(payloadEndpointKey, new TextEncoder().encode(JWT_SERVER_KEY), {
        maxTokenAge: '6 minutes'
      });
    } catch (e) {
      return await util.unsuccessfulResponse(`Your session has expired (0x00004001) | ID: ${clientRay}`);
    }

    if (decodedEndpointKey.payload.endpoint !== 'fetchv2' || !decodedEndpointKey.payload.username || decodedEndpointKey.payload.username === 'null') {
      util.sentryMessage(`0x00004002: Failed check: ${decodedEndpointKey.payload.endpoint}, ${decodedEndpointKey.payload.username}`, sentry, util.SENTRY_LEVELS.WARNING);
      return await util.unsuccessfulResponse(`Something went wrong (0x00004002) | ID: ${clientRay}`);
    }

    let bin = null;

    // overwrite the bin path based on their usergroup
    // STRIPPED_FOR_PUBLIC_RELEASE users
    if (decodedEndpointKey.payload.branch === 'STRIPPED_FOR_PUBLIC_RELEASE' && decodedEndpointKey.payload.packages.includes(8)) {
      bin = 'STRIPPED_FOR_PUBLIC_RELEASE/STRIPPED_FOR_PUBLIC_RELEASE.bin';
    }
    // STRIPPED_FOR_PUBLIC_RELEASE
    else if (decodedEndpointKey.payload.branch === 'STRIPPED_FOR_PUBLIC_RELEASE' && decodedEndpointKey.payload.packages.includes(7)) {
      bin = 'STRIPPED_FOR_PUBLIC_RELEASE/STRIPPED_FOR_PUBLIC_RELEASE.bin';
    }
    // STRIPPED_FOR_PUBLIC_RELEASE not supported at this time
    else if (decodedEndpointKey.payload.branch === 'STRIPPED_FOR_PUBLIC_RELEASE' && decodedEndpointKey.payload.packages.includes(6)) {
      bin = 'STRIPPED_FOR_PUBLIC_RELEASE/STRIPPED_FOR_PUBLIC_RELEASE.bin';
    }
    // flag this, how did they even get here?
    else {
      util.sentryMessage(`0x00004003: Unauthorized branch access`, sentry, util.SENTRY_LEVELS.ERROR);
      return await util.unsuccessfulResponse(`Something went wrong (0x00004003) | ID: ${clientRay}`);
    }
    
    const stream = await fetch(`https://cdn2.STRIPPED_FOR_PUBLIC_RELEASE.com/data/assets/${bin}`, {
      headers: {
        'user-agent': 'STRIPPED_FOR_PUBLIC_RELEASE_CF_WORKER'
      }
    });

    if (stream.status !== 200) {
      util.sentryMessage(`0x00004004: Stream fetch ${bin} failed status ${stream.status}`, sentry, util.SENTRY_LEVELS.ERROR);
      return await util.unsuccessfulResponse(`Something went wrong (0x00004004) | ID: ${clientRay}`);
    }

    const blob = (await stream.blob()).stream();
    return new Response(
      blob,
      {
        status: 200,
        headers: { 'content-type': 'application/octet-stream' }
      }
    );
  }
  catch (e) {
    util.sentryError(e.message, sentry);
    return new Response(null, { status: 500 });
  }
}