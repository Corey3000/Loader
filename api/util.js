export async function mutateResponse(message) {
  let random = crypto.randomUUID();
  // base64 it first
  message = btoa(message);
  // add random chars to it
  message = message.replace(/.{1}/g, function(m) {
      return m + random[Math.floor(Math.random() * 30)]
  });
  // add at first index
  message = random[Math.floor(Math.random() * 30)] + message;
  // put the response in reverse
  return message.split("").reverse().join("");
}

export async function decryptRequest(message) {
  message = message.split("").reverse().join("");
  let parsedMessage = "";

  for (let i = 0; i < message.length; i++) {
    if(i % 2 == 0) {
        continue;
    }

    parsedMessage = parsedMessage.concat(message[i]);
  }

  return atob(parsedMessage);
}

export async function unsuccessfulResponse(message) {
  try {
    var response = {
      'success': false,
      'response': {
        'message': message
      }
    };

    response = JSON.stringify(response);

    return new Response(await mutateResponse(response), { status: 400 });
  }
    catch(e) {
    console.error(e.message);
    return new Response(null, { status: 500 } );
  }
}
  
export async function successfulResponse(endpoint, endpointKey, paintStage, level = '0', extras = 'null') {
  try {
    var response = {
      'success': true,
      'has_extras': extras === 'null' ? false : true,
      'response': {
        'endpoint': `${endpoint}`,
        'endpoint_key': `${endpointKey}`,
        'paint_stage': `${paintStage}`,
        'level': level,
        'extras': `${extras}`,
        'extras_key': extras === 'null' ? 'null' : '134250805'
      }
    };
  
    response = JSON.stringify(response);

    console.log(response);

    return new Response(await mutateResponse(response), { status: 200 });
  }
  catch(e) {
    console.error(e.message);
    return new Response(null, { status: 500 } );
  }
}
  
export async function hash32(str) {
  try {
    var hash = 0x811c9dc5;

    for( var i = 0; i < str.length; ++i ) {
      hash ^= str.charCodeAt(i);
      hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);
    }

    return hash >>> 0;
  }
  catch(e) {
    console.error(e.message);
    return null;
  }
}

export async function sha256hash(str) {
  str = new TextEncoder().encode(str);

  const digest = await crypto.subtle.digest(
    {
      name: 'SHA-256'
    },
    str
  );

  const hexString = [...new Uint8Array(digest)]
    .map(b => b.toString(16).padStart(2, '0'))
    .join('');

  return hexString;
}

export async function createOffsets(offsets, legacySupport) {
  async function createHashPair(key, str) {
    offsets['response'][key][await hash32(str)] = await mutateResponse(str);
  }
  // the module scanner on the client takes in a hash, with the clients existing system this would not work as the client would know the hash. 
  // by mapping it at a different index and giving those indexes to the client, we can depend on the server for the values
  // we should ONLY do this for modules as its quite custom
  async function createModulePair(key, str, index) {
    offsets['response'][key][await hash32(index)] = await hash32(str);
  }

  if (legacySupport) {
    // STRIPPED_FOR_PUBLIC_RELEASE
  } else {
    // STRIPPED_FOR_PUBLIC_RELEASE
  }

  return offsets;
}

export const SENTRY_LEVELS = {
  INFO: 'info',
  WARNING: 'warning',
  ERROR: 'error'
};

export function sentryMessage(message, sentry, sentryLevel = SENTRY_LEVELS.INFO) {
  console.log(`[${sentryLevel}]: ${message}`);
  sentry.captureMessage(message, sentryLevel);
}

export function sentryError(message, sentry) {
  console.error(message);
  sentry.captureException(message);
}