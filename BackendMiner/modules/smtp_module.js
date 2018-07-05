//smtp_module.js 
//create on 2018-06-14 by wjun

const nodemailer = require('nodemailer');
var configHandle = require('./config_module');

//Create Smtp
function BwSmtpCreate(host,port,secure,user,pass,from,to)
{
    let transporter = nodemailer.createTransport({
        host: host,
        secureConnection: secure, // use SSL
        port: port,
        secure: secure, // secure:true for port 465, secure:false for port 587
        auth: {
            user: user,//user
            pass: pass //授权码
        }
    });

    return transporter;
}


//Send mail
function BwSmtpSendMail(transporter,from,to,msg)
{
    let mailOptions = {
        from: from, //发件人
        to: to, // 收件人
        subject: 'Pass告警', //主题
        html:`<h2>告警详情见附件:</h2>`, // html body
        attachments:[
            {
              filename : 'mail_attach',
              path: configHandle.MAILFILE,
            },  
          ]
    };
    if (transporter){
        transporter.sendMail(mailOptions, (error, info) => {
            if (error) {
                return console.log(error);
            }
            console.log(`Message: ${info.messageId}`);
            console.log(`sent: ${info.response}`);
        });
    }
}

exports.BwSmtpCreate = BwSmtpCreate;
exports.BwSmtpSendMail = BwSmtpSendMail;